#include "i3space/i3_ipc.hpp"
#include "i3space/workspace_view.hpp"

#include <iostream>
#include <string>

namespace i3space {

int print_active_main(const std::string& filter_output) {
  I3Ipc ipc;

  std::string ws_json;
  std::string tree_json;
  std::string out_json;
  if (!ipc.get_workspaces(ws_json)) {
    std::cerr << "i3space: get_workspaces failed: " << ipc.last_error() << "\n";
    return 1;
  }
  if (!ipc.get_tree(tree_json)) {
    std::cerr << "i3space: get_tree failed: " << ipc.last_error() << "\n";
    return 1;
  }
  if (!ipc.get_outputs(out_json)) {
    std::cerr << "i3space: get_outputs failed: " << ipc.last_error() << "\n";
    return 1;
  }

  WorkspaceView view;
  std::string err;
  if (!flatten_i3_tree(ws_json, tree_json, out_json, view, err)) {
    std::cerr << "i3space: " << err << "\n";
    return 1;
  }

  bool first_output = true;
  for (const auto& og : view.outputs) {
    if (!filter_output.empty() && og.name != filter_output) {
      continue;
    }
    if (!first_output) {
      std::cout << ',';
    }
    first_output = false;
    std::cout << "\n  \"" << og.name << "\": [";
    bool first_ws = true;
    for (int idx : og.ws_indices) {
      if (idx < 0 || idx >= static_cast<int>(view.workspaces.size())) {
        continue;
      }
      const auto& ws = view.workspaces[static_cast<size_t>(idx)];
      if (!first_ws) {
        std::cout << ',';
      }
      first_ws = false;
      std::cout << "\n    {\"num\":" << ws.num << ",\"name\":\"" << ws.name
                << "\",\"windows\":" << ws.win_count
                << ",\"urgent\":" << static_cast<int>(ws.urgent)
                << ",\"focused\":" << static_cast<int>(ws.focused) << '}';
    }
    if (!first_ws) {
      std::cout << '\n';
    }
    std::cout << "  ]";
  }
  if (!first_output) {
    std::cout << '\n';
  }
  std::cout << std::flush;
  return 0;
}

}  // namespace i3space
