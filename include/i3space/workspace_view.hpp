// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cat-Not-Furry

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace i3space {

struct WindowEntry {
  int ws_index{-1};
  int rect_x{0};
  int rect_y{0};
  int rect_w{0};
  int rect_h{0};
  float norm_x{0.f};
  float norm_y{0.f};
  float norm_w{0.f};
  float norm_h{0.f};
  std::string title;
  uint8_t focused{0};
};

struct WorkspaceEntry {
  int num{0};
  std::string name;
  std::string output;
  uint8_t urgent{0};
  uint8_t focused{0};
  uint8_t visible{0};
  int first_win{-1};
  int win_count{0};
};

struct OutputGroup {
  std::string name;
  int x{0};
  int y{0};
  int width{0};
  int height{0};
  std::vector<int> ws_indices;
};

struct WorkspaceView {
  std::vector<WorkspaceEntry> workspaces;
  std::vector<WindowEntry> windows;
  std::vector<OutputGroup> outputs;
  std::string string_pool;

  void clear();
  int workspace_count_for_output(const std::string& output) const;
  const OutputGroup* find_output(const std::string& name) const;
};

bool flatten_i3_tree(const std::string& workspaces_json,
                       const std::string& tree_json,
                       const std::string& outputs_json,
                       WorkspaceView& out,
                       std::string& error_out);

}  // namespace i3space
