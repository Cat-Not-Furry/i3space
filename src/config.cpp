// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#include "i3space/config.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

namespace i3space {

namespace {

std::string trim(std::string s) {
  while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
    s.erase(s.begin());
  }
  while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) {
    s.pop_back();
  }
  return s;
}

enum class Section { None, Config, Transparency };

void apply_kv(const std::string& key, const std::string& val, Config& cfg,
              Section sec) {
  ConfigSection* target = &cfg.config;
  if (sec == Section::Transparency) {
    target = &cfg.transparency;
  } else if (sec == Section::None) {
    return;
  }

  auto parse_int = [&](int& out) {
    try {
      out = std::stoi(val);
    } catch (...) {
    }
  };
  auto parse_double = [&](double& out) {
    try {
      out = std::stod(val);
    } catch (...) {
    }
  };

  if (key == "bar_margin_px") {
    parse_int(target->bar_margin_px);
  } else if (key == "card_width") {
    parse_int(target->card_width);
  } else if (key == "card_height") {
    parse_int(target->card_height);
  } else if (key == "strip_margin_px") {
    parse_int(target->strip_margin_px);
  } else if (key == "single_ws_toast_ms") {
    parse_int(target->single_ws_toast_ms);
  } else if (key == "single_ws_message") {
    target->single_ws_message = val;
  } else if (key == "strip_position") {
    target->strip_position = val;
  } else if (key == "opacity" || key == "picom_opacity") {
    parse_double(target->opacity);
  } else if (key == "blur_background") {
    target->blur_background = (val == "1" || val == "true" || val == "yes");
  } else if (key == "blur_strength") {
    parse_int(target->blur_strength);
  } else if (key == "fade_ms") {
    parse_int(target->fade_ms);
  } else if (key == "scroll_step_px") {
    parse_int(target->scroll_step_px);
  }
}

}  // namespace

std::string Config::config_dir() {
  if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) {
    return std::string(xdg) + "/i3space";
  }
  if (const char* home = std::getenv("HOME")) {
    return std::string(home) + "/.config/i3space";
  }
  return "/tmp/i3space";
}

std::string Config::default_config_path() {
  return config_dir() + "/i3space";
}

bool parse_config_file(const std::string& path, Config& out, std::string& error_out) {
  std::ifstream in(path);
  if (!in) {
    error_out = "cannot open " + path;
    return false;
  }

  Section sec = Section::None;
  std::string line;
  while (std::getline(in, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#') {
      if (line == "#!config") {
        sec = Section::Config;
      } else if (line == "#!transparency") {
        sec = Section::Transparency;
      }
      continue;
    }
    const auto eq = line.find('=');
    if (eq == std::string::npos) {
      continue;
    }
    const std::string key = trim(line.substr(0, eq));
    const std::string val = trim(line.substr(eq + 1));
    apply_kv(key, val, out, sec);
  }
  out.loaded_path = path;
  return true;
}

bool write_default_config(const std::string& path) {
  const fs::path p(path);
  std::error_code ec;
  fs::create_directories(p.parent_path(), ec);

  std::ifstream example_in(fs::path(__FILE__).parent_path().parent_path() /
                           "examples/i3space.default");
  std::ofstream out(path);
  if (!out) {
    return false;
  }
  if (example_in) {
    out << example_in.rdbuf();
  } else {
    out << "#!config\n"
        << "bar_margin_px = 0\n"
        << "card_width = 255\n"
        << "card_height = 180\n"
        << "strip_margin_px = 8\n"
        << "single_ws_toast_ms = 2000\n"
        << "single_ws_message =\n"
        << "\n#!transparency\n"
        << "opacity = 0.92\n"
        << "blur_background = false\n"
        << "blur_strength = 5\n"
        << "fade_ms = 0\n"
        << "scroll_step_px = 64\n";
  }
  return static_cast<bool>(out);
}

Config Config::load(std::string& error_out) {
  Config cfg;
  const std::string unified = default_config_path();
  const std::string unified_conf = unified + ".conf";

  std::vector<std::string> paths;
  if (fs::exists(unified)) {
    paths.push_back(unified);
  }
  if (fs::exists(unified_conf)) {
    paths.push_back(unified_conf);
  }

  if (const char* home = std::getenv("HOME")) {
    const std::string i3_path = std::string(home) + "/.config/i3/i3space.conf";
    const std::string i3_plain = std::string(home) + "/.config/i3/i3space";
    const std::string picom_path = std::string(home) + "/.config/picom/i3space.conf";
    if (paths.empty()) {
      if (fs::exists(i3_path)) {
        paths.push_back(i3_path);
      } else if (fs::exists(i3_plain)) {
        paths.push_back(i3_plain);
      }
      if (fs::exists(picom_path)) {
        paths.push_back(picom_path);
      }
    }
  }

  if (paths.empty()) {
    cfg.loaded_path = "(defaults)";
    return cfg;
  }

  for (const auto& path : paths) {
    std::string err;
    Config partial;
    if (!parse_config_file(path, partial, err)) {
      error_out = err;
      continue;
    }
    if (cfg.loaded_path.empty()) {
      cfg = partial;
    } else {
      if (!partial.config.single_ws_message.empty()) {
        cfg.config.single_ws_message = partial.config.single_ws_message;
      }
      cfg.transparency.opacity = partial.transparency.opacity;
      cfg.transparency.blur_background = partial.transparency.blur_background;
      cfg.transparency.blur_strength = partial.transparency.blur_strength;
      cfg.transparency.fade_ms = partial.transparency.fade_ms;
      cfg.transparency.scroll_step_px = partial.transparency.scroll_step_px;
    }
  }
  return cfg;
}

}  // namespace i3space
