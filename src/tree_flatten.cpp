#include "i3space/workspace_view.hpp"

#include <simdjson.h>

#include <algorithm>
#include <unordered_map>

namespace i3space {

void WorkspaceView::clear() {
  workspaces.clear();
  windows.clear();
  outputs.clear();
  string_pool.clear();
}

int WorkspaceView::workspace_count_for_output(const std::string& output) const {
  int n = 0;
  for (const auto& ws : workspaces) {
    if (ws.output == output) {
      ++n;
    }
  }
  return n;
}

const OutputGroup* WorkspaceView::find_output(const std::string& name) const {
  for (const auto& o : outputs) {
    if (o.name == name) {
      return &o;
    }
  }
  return nullptr;
}

namespace {

bool parse_outputs(const std::string& json, WorkspaceView& view) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded(json);
  simdjson::ondemand::document doc;
  if (parser.iterate(padded).get(doc)) {
    return false;
  }
  simdjson::ondemand::array arr;
  if (doc.get_array().get(arr)) {
    return false;
  }
  for (auto elem_result : arr) {
    simdjson::ondemand::value elem;
    if (elem_result.get(elem)) {
      continue;
    }
    OutputGroup og;
    std::string_view name;
    if (elem["name"].get_string().get(name)) {
      continue;
    }
    og.name = std::string(name);
    simdjson::ondemand::object rect;
    if (!elem["rect"].get_object().get(rect)) {
      view.outputs.push_back(std::move(og));
      continue;
    }
    int64_t v = 0;
    if (!rect["x"].get(v)) {
      og.x = static_cast<int>(v);
    }
    if (!rect["y"].get(v)) {
      og.y = static_cast<int>(v);
    }
    if (!rect["width"].get(v)) {
      og.width = static_cast<int>(v);
    }
    if (!rect["height"].get(v)) {
      og.height = static_cast<int>(v);
    }
    view.outputs.push_back(std::move(og));
  }
  return true;
}

void walk_tree(simdjson::ondemand::value node, const std::string& current_output,
               const std::string& current_ws,
               std::unordered_map<std::string, int>& ws_name_to_idx,
               WorkspaceView& view) {
  std::string_view type_sv;
  if (node["type"].get_string().get(type_sv)) {
    return;
  }
  const std::string type(type_sv);

  std::string output = current_output;
  std::string ws_name = current_ws;

  if (type == "output") {
    std::string_view name;
    if (!node["name"].get_string().get(name)) {
      output = std::string(name);
    }
  } else if (type == "workspace") {
    std::string_view name;
    if (!node["name"].get_string().get(name)) {
      ws_name = std::string(name);
    }
  } else if (node["window"].error() == simdjson::SUCCESS) {
    if (!ws_name.empty()) {
      auto it = ws_name_to_idx.find(ws_name);
      if (it != ws_name_to_idx.end()) {
        WindowEntry we;
        we.ws_index = it->second;
        std::string_view title;
        if (!node["name"].get_string().get(title)) {
          we.title = std::string(title);
        }
        simdjson::ondemand::object rect;
        if (!node["rect"].get_object().get(rect)) {
          int64_t v = 0;
          if (!rect["x"].get(v)) {
            we.rect_x = static_cast<int>(v);
          }
          if (!rect["y"].get(v)) {
            we.rect_y = static_cast<int>(v);
          }
          if (!rect["width"].get(v)) {
            we.rect_w = static_cast<int>(v);
          }
          if (!rect["height"].get(v)) {
            we.rect_h = static_cast<int>(v);
          }
        }
        bool focused = false;
        if (!node["focused"].get(focused) && focused) {
          we.focused = 1;
        }
        auto& ws = view.workspaces[static_cast<size_t>(we.ws_index)];
        const int win_idx = static_cast<int>(view.windows.size());
        if (ws.first_win < 0) {
          ws.first_win = win_idx;
        }
        ++ws.win_count;
        view.windows.push_back(std::move(we));
      }
    }
    return;
  }

  simdjson::ondemand::array nodes;
  if (node["nodes"].get_array().get(nodes)) {
    return;
  }
  for (auto child_result : nodes) {
    simdjson::ondemand::value child;
    if (child_result.get(child)) {
      continue;
    }
    walk_tree(child, output, ws_name, ws_name_to_idx, view);
  }
}

}  // namespace

bool flatten_i3_tree(const std::string& workspaces_json,
                     const std::string& tree_json,
                     const std::string& outputs_json,
                     WorkspaceView& out,
                     std::string& error_out) {
  out.clear();

  simdjson::ondemand::parser ws_parser;
  simdjson::padded_string ws_padded(workspaces_json);
  simdjson::ondemand::document ws_doc;
  if (ws_parser.iterate(ws_padded).get(ws_doc)) {
    error_out = "parse get_workspaces failed";
    return false;
  }

  simdjson::ondemand::array ws_arr;
  if (ws_doc.get_array().get(ws_arr)) {
    error_out = "get_workspaces is not an array";
    return false;
  }

  std::unordered_map<std::string, int> ws_name_to_idx;
  for (auto elem_result : ws_arr) {
    simdjson::ondemand::value elem;
    if (elem_result.get(elem)) {
      continue;
    }
    WorkspaceEntry we;
    int64_t num = 0;
    if (!elem["num"].get(num)) {
      we.num = static_cast<int>(num);
    }
    std::string_view name;
    if (!elem["name"].get_string().get(name)) {
      we.name = std::string(name);
    }
    std::string_view output;
    if (!elem["output"].get_string().get(output)) {
      we.output = std::string(output);
    }
    bool urgent = false;
    if (!elem["urgent"].get(urgent) && urgent) {
      we.urgent = 1;
    }
    bool focused = false;
    if (!elem["focused"].get(focused) && focused) {
      we.focused = 1;
    }
    bool visible = false;
    if (!elem["visible"].get(visible) && visible) {
      we.visible = 1;
    }
    ws_name_to_idx[we.name] = static_cast<int>(out.workspaces.size());
    out.workspaces.push_back(std::move(we));
  }

  if (!parse_outputs(outputs_json, out)) {
    for (auto& ws : out.workspaces) {
      bool found = false;
      for (auto& og : out.outputs) {
        if (og.name == ws.output) {
          og.ws_indices.push_back(static_cast<int>(&ws - out.workspaces.data()));
          found = true;
          break;
        }
      }
      if (!found) {
        OutputGroup og;
        og.name = ws.output;
        og.ws_indices.push_back(static_cast<int>(&ws - out.workspaces.data()));
        out.outputs.push_back(std::move(og));
      }
    }
  } else {
    for (size_t i = 0; i < out.workspaces.size(); ++i) {
      for (auto& og : out.outputs) {
        if (og.name == out.workspaces[i].output) {
          og.ws_indices.push_back(static_cast<int>(i));
          break;
        }
      }
    }
  }

  simdjson::ondemand::parser tree_parser;
  simdjson::padded_string tree_padded(tree_json);
  simdjson::ondemand::document tree_doc;
  if (tree_parser.iterate(tree_padded).get(tree_doc)) {
    error_out = "parse get_tree failed";
    return false;
  }

  walk_tree(tree_doc, "", "", ws_name_to_idx, out);

  for (auto& we : out.windows) {
    if (we.ws_index < 0 || we.ws_index >= static_cast<int>(out.workspaces.size())) {
      continue;
    }
    const auto& siblings = out.workspaces[static_cast<size_t>(we.ws_index)];
    int min_x = we.rect_x;
    int min_y = we.rect_y;
    int max_x = we.rect_x + we.rect_w;
    int max_y = we.rect_y + we.rect_h;
    for (int i = 0; i < siblings.win_count; ++i) {
      const auto& w = out.windows[static_cast<size_t>(siblings.first_win + i)];
      min_x = std::min(min_x, w.rect_x);
      min_y = std::min(min_y, w.rect_y);
      max_x = std::max(max_x, w.rect_x + w.rect_w);
      max_y = std::max(max_y, w.rect_y + w.rect_h);
    }
    const int bw = std::max(1, max_x - min_x);
    const int bh = std::max(1, max_y - min_y);
    we.norm_x = static_cast<float>(we.rect_x - min_x) / static_cast<float>(bw);
    we.norm_y = static_cast<float>(we.rect_y - min_y) / static_cast<float>(bh);
    we.norm_w = static_cast<float>(we.rect_w) / static_cast<float>(bw);
    we.norm_h = static_cast<float>(we.rect_h) / static_cast<float>(bh);
  }

  return true;
}

}  // namespace i3space
