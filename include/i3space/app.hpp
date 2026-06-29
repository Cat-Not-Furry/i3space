// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#pragma once

#include <string>

namespace i3space {

int print_active_main(const std::string& filter_output);
int toggle_main();

void print_help(const char* argv0);
void print_version();

}  // namespace i3space
