#pragma once

#include "i3space/config.hpp"
#include "i3space/workspace_view.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string>
#include <vector>

namespace i3space {

struct CardLayout {
  int ws_index{-1};
  int x{0};
  int y{0};
  int w{0};
  int h{0};
};

struct OutputStrip {
  std::string output_name;
  int output_x{0};
  int output_y{0};
  int output_w{0};
  int output_h{0};
  Window window{0};
  Pixmap pixmap{0};
  int band_y{0};
  int band_h{0};
  int scroll_x{0};
  std::vector<CardLayout> cards;
};

class StripSession {
 public:
  explicit StripSession(const Config& cfg);
  ~StripSession();

  bool open(const WorkspaceView& view);
  void close();
  bool is_open() const { return display_ != nullptr; }

  int run_loop(const WorkspaceView& view, std::string& selected_ws_out);

  Display* display() const { return display_; }

 private:
  Config cfg_;
  Display* display_{nullptr};
  int screen_{0};
  Visual* visual_{nullptr};
  Colormap cmap_{0};
  int depth_{0};
  std::vector<OutputStrip> strips_;
  int selected_card_{0};
  int active_output_{0};

  bool setup_display();
  void build_strips(const WorkspaceView& view);
  void draw_strip(size_t output_index, OutputStrip& strip, const WorkspaceView& view);
  void redraw_all(const WorkspaceView& view);
  bool select_card(int global_index);
  int card_count() const;
  int global_to_local(int global, int& out_output, int& out_local) const;
  int local_to_global(int output, int local) const;
  void ensure_card_visible(OutputStrip& strip, int local_index);
  void apply_opacity(Window win);
};

int show_single_ws_toast(const Config& cfg, const OutputGroup& output,
                         Display* display);

}  // namespace i3space
