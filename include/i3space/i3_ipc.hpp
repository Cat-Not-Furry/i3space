// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace i3space {

enum class IpcMessageType : uint32_t {
  RunCommand = 0,
  GetWorkspaces = 1,
  Subscribe = 2,
  GetOutputs = 3,
  GetTree = 4,
};

class I3Ipc {
 public:
  I3Ipc();
  ~I3Ipc();

  I3Ipc(const I3Ipc&) = delete;
  I3Ipc& operator=(const I3Ipc&) = delete;

  bool connect();
  bool is_connected() const { return fd_ >= 0; }
  std::string last_error() const { return last_error_; }

  bool request(IpcMessageType type, const std::string& payload, std::string& reply_out);
  bool run_command(const std::string& command);
  bool get_workspaces(std::string& json_out);
  bool get_tree(std::string& json_out);
  bool get_outputs(std::string& json_out);

  static std::string resolve_socket_path();

 private:
  int fd_{-1};
  std::string last_error_;

  void disconnect();
  bool write_message(IpcMessageType type, const std::string& payload);
  bool read_reply(std::string& reply_out, uint32_t& reply_type_out);
};

}  // namespace i3space
