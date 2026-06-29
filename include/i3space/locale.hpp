// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#pragma once

namespace i3space {

enum class Locale { En, Es };

Locale detect_locale();
const char* msg(Locale loc, const char* en, const char* es);
bool ensure_first_run();

}  // namespace i3space
