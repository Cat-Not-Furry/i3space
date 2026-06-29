// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#pragma once

#include <string>

namespace i3space {

struct ConfigSection {
  int bar_margin_px{0};
  int card_width{255};
  int card_height{180};
  int strip_margin_px{8};
  int single_ws_toast_ms{2000};
  std::string single_ws_message;
  std::string strip_position{"top"};

  double opacity{0.92};
  bool blur_background{false};
  int blur_strength{5};
  int fade_ms{0};
  int scroll_step_px{64};
};

struct Config {
  ConfigSection config;
  ConfigSection transparency;
  std::string loaded_path;

  static Config load(std::string& error_out);
  static std::string default_config_path();
  static std::string config_dir();
};

bool parse_config_file(const std::string& path, Config& out, std::string& error_out);
bool write_default_config(const std::string& path);

}  // namespace i3space
