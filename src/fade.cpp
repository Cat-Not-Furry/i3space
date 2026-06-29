// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#include "i3space/fade.hpp"

#include <chrono>
#include <thread>

namespace i3space {

void fade_delay_ms(int ms) {
  if (ms <= 0) {
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

}  // namespace i3space
