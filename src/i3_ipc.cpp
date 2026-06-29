#include "i3space/i3_ipc.hpp"

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace i3space {

namespace {

constexpr std::array<char, 6> kMagic{'i', '3', '-', 'i', 'p', 'c'};
constexpr int kIpcTimeoutMs = 10000;

#if defined(__GNUC__) || defined(__clang__)
#define I3SPACE_PACKED __attribute__((packed))
#else
#define I3SPACE_PACKED
#endif

struct I3SPACE_PACKED IpcHeader {
  char magic[6];
  uint32_t size;
  uint32_t type;
};

static_assert(sizeof(IpcHeader) == 14, "i3 IPC header must be exactly 14 bytes");

#undef I3SPACE_PACKED

bool poll_readable(int fd, int timeout_ms) {
  pollfd pfd{};
  pfd.fd = fd;
  pfd.events = POLLIN;
  while (true) {
    const int r = ::poll(&pfd, 1, timeout_ms);
    if (r < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    if (r == 0) {
      errno = ETIMEDOUT;
      return false;
    }
    return true;
  }
}

bool read_exact(int fd, void* buf, size_t len) {
  auto* p = static_cast<char*>(buf);
  size_t got = 0;
  while (got < len) {
    if (!poll_readable(fd, kIpcTimeoutMs)) {
      return false;
    }
    const ssize_t n = ::read(fd, p + got, len - got);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    if (n == 0) {
      return false;
    }
    got += static_cast<size_t>(n);
  }
  return true;
}

bool write_exact(int fd, const void* buf, size_t len) {
  const auto* p = static_cast<const char*>(buf);
  size_t sent = 0;
  while (sent < len) {
    const ssize_t n = ::write(fd, p + sent, len - sent);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    sent += static_cast<size_t>(n);
  }
  return true;
}

}  // namespace

I3Ipc::I3Ipc() = default;

I3Ipc::~I3Ipc() {
  disconnect();
}

void I3Ipc::disconnect() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

std::string I3Ipc::resolve_socket_path() {
  if (const char* env = std::getenv("I3SOCK")) {
    if (env[0] != '\0') {
      return env;
    }
  }
  FILE* fp = popen("i3 --get-socketpath 2>/dev/null", "r");
  if (!fp) {
    return {};
  }
  char buf[512]{};
  if (!std::fgets(buf, sizeof(buf), fp)) {
    pclose(fp);
    return {};
  }
  pclose(fp);
  std::string path(buf);
  while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
    path.pop_back();
  }
  return path;
}

bool I3Ipc::connect() {
  if (fd_ >= 0) {
    return true;
  }
  const std::string path = resolve_socket_path();
  if (path.empty()) {
    last_error_ = "cannot resolve i3 IPC socket (I3SOCK / i3 --get-socketpath)";
    return false;
  }

  fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd_ < 0) {
    last_error_ = "socket() failed";
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  if (path.size() >= sizeof(addr.sun_path)) {
    last_error_ = "socket path too long";
    disconnect();
    return false;
  }
  std::memcpy(addr.sun_path, path.c_str(), path.size() + 1);

  if (::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
    last_error_ = "connect() to i3 IPC failed";
    disconnect();
    return false;
  }
  return true;
}

bool I3Ipc::write_message(IpcMessageType type, const std::string& payload) {
  IpcHeader hdr{};
  std::memcpy(hdr.magic, kMagic.data(), kMagic.size());
  hdr.size = static_cast<uint32_t>(payload.size());
  hdr.type = static_cast<uint32_t>(type);

  if (!write_exact(fd_, &hdr, sizeof(hdr))) {
    last_error_ = "write header failed";
    return false;
  }
  if (!payload.empty() && !write_exact(fd_, payload.data(), payload.size())) {
    last_error_ = "write payload failed";
    return false;
  }
  return true;
}

bool I3Ipc::read_reply(std::string& reply_out, uint32_t& reply_type_out) {
  IpcHeader hdr{};
  if (!read_exact(fd_, &hdr, sizeof(hdr))) {
    if (errno == ETIMEDOUT) {
      last_error_ = "read header timed out (is i3 running?)";
    } else {
      last_error_ = "read header failed";
    }
    return false;
  }
  if (std::memcmp(hdr.magic, kMagic.data(), kMagic.size()) != 0) {
    last_error_ = "invalid IPC magic";
    return false;
  }
  reply_type_out = hdr.type;
  reply_out.resize(hdr.size);
  if (hdr.size > 0 && !read_exact(fd_, reply_out.data(), hdr.size)) {
    if (errno == ETIMEDOUT) {
      last_error_ = "read payload timed out";
    } else {
      last_error_ = "read payload failed";
    }
    return false;
  }
  return true;
}

bool I3Ipc::request(IpcMessageType type, const std::string& payload,
                     std::string& reply_out) {
  disconnect();
  if (!connect()) {
    return false;
  }
  if (!write_message(type, payload)) {
    disconnect();
    return false;
  }
  uint32_t reply_type = 0;
  const bool ok = read_reply(reply_out, reply_type);
  disconnect();
  return ok;
}

bool I3Ipc::run_command(const std::string& command) {
  std::string reply;
  return request(IpcMessageType::RunCommand, command, reply);
}

bool I3Ipc::get_workspaces(std::string& json_out) {
  return request(IpcMessageType::GetWorkspaces, "", json_out);
}

bool I3Ipc::get_tree(std::string& json_out) {
  return request(IpcMessageType::GetTree, "", json_out);
}

bool I3Ipc::get_outputs(std::string& json_out) {
  return request(IpcMessageType::GetOutputs, "", json_out);
}

}  // namespace i3space
