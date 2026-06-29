#include "i3space/i3_ipc.hpp"
#include "i3space/config.hpp"
#include "i3space/workspace_view.hpp"
#include "i3space/x11_strip.hpp"

#include <iostream>

namespace i3space {

int toggle_main() {
  std::string cfg_err;
  const Config cfg = Config::load(cfg_err);
  if (!cfg_err.empty()) {
    std::cerr << "i3space: config warning: " << cfg_err << "\n";
  }

  I3Ipc ipc;
  if (!ipc.connect()) {
    std::cerr << "i3space: " << ipc.last_error() << "\n";
    return 1;
  }

  std::string ws_json;
  std::string tree_json;
  std::string out_json;
  if (!ipc.get_workspaces(ws_json) || !ipc.get_tree(tree_json) ||
      !ipc.get_outputs(out_json)) {
    std::cerr << "i3space: IPC failed\n";
    return 1;
  }

  WorkspaceView view;
  std::string err;
  if (!flatten_i3_tree(ws_json, tree_json, out_json, view, err)) {
    std::cerr << "i3space: " << err << "\n";
    return 1;
  }

  int multi_count = 0;
  for (const auto& og : view.outputs) {
    const int n = view.workspace_count_for_output(og.name);
    if (n < 2) {
      if (n == 1) {
        show_single_ws_toast(cfg, og, nullptr);
      }
      continue;
    }
    multi_count += n;
  }

  if (multi_count == 0) {
    return 0;
  }

  StripSession session(cfg);
  if (!session.open(view)) {
    std::cerr << "i3space: cannot open overview (need 2+ workspaces on an output)\n";
    return 1;
  }

  std::string selected_ws;
  const int rc = session.run_loop(view, selected_ws);
  session.close();

  if (rc == 0 && !selected_ws.empty()) {
    bool numeric = !selected_ws.empty();
    for (char c : selected_ws) {
      if (c < '0' || c > '9') {
        numeric = false;
        break;
      }
    }
    if (numeric) {
      ipc.run_command("workspace " + selected_ws);
    } else {
      ipc.run_command("workspace \"" + selected_ws + "\"");
    }
  }
  return rc == 2 ? 0 : rc;
}

}  // namespace i3space
