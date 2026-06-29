// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#include "i3space/compositor_detect.hpp"

#include <array>
#include <cstdio>
#include <string>

namespace i3space {

namespace {

bool pgrep_name(const char* name) {
  std::string cmd = "pgrep -x ";
  cmd += name;
  cmd += " >/dev/null 2>&1";
  return std::system(cmd.c_str()) == 0;
}

}  // namespace

bool compositor_running() {
  static constexpr std::array<const char*, 5> kNames{
      "picom", "compton", "xcompmgr", "picom-trans", "compositor"};
  for (const char* n : kNames) {
    if (pgrep_name(n)) {
      return true;
    }
  }
  return false;
}

std::string compositor_name() {
  static constexpr std::array<const char*, 5> kNames{
      "picom", "compton", "xcompmgr", "picom-trans", "compositor"};
  for (const char* n : kNames) {
    if (pgrep_name(n)) {
      return n;
    }
  }
  return {};
}

}  // namespace i3space
