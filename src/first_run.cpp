// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#include "i3space/locale.hpp"
#include "i3space/config.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;

namespace i3space {

namespace {

bool is_initialized() {
  const std::string flag = Config::config_dir() + "/.initialized";
  return fs::exists(flag);
}

void mark_initialized() {
  const std::string flag = Config::config_dir() + "/.initialized";
  std::ofstream out(flag);
  out << "1\n";
}

bool prompt_yes_no(Locale loc) {
  std::cout << msg(loc,
                   "Use default configuration? [Y/n]: ",
                   "¿Usar configuración por defecto? [S/n]: ");
  std::string line;
  if (!std::getline(std::cin, line)) {
    return true;
  }
  if (line.empty()) {
    return true;
  }
  return line[0] == 'Y' || line[0] == 'y' || line[0] == 'S' || line[0] == 's';
}

}  // namespace

bool ensure_first_run() {
  const std::string cfg_path = Config::default_config_path();
  if (fs::exists(cfg_path) && is_initialized()) {
    return true;
  }

  const Locale loc = detect_locale();
  const std::string dir = Config::config_dir();

  if (isatty(STDIN_FILENO)) {
    std::cout << msg(loc,
                     "Welcome to i3space.\n"
                     "Configuration is stored in ~/.config/i3space/\n",
                     "Bienvenido a i3space.\n"
                     "La configuración se guarda en ~/.config/i3space/\n");
    if (!prompt_yes_no(loc)) {
      std::cout << msg(loc,
                       "Create your config at ~/.config/i3space/i3space and run again.\n",
                       "Crea tu config en ~/.config/i3space/i3space y vuelve a ejecutar.\n");
      mark_initialized();
      return false;
    }
  }

  if (!write_default_config(cfg_path)) {
    std::cerr << "i3space: failed to write default config\n";
    return false;
  }
  mark_initialized();
  return true;
}

}  // namespace i3space
