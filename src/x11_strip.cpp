#include "i3space/x11_strip.hpp"

#include "i3space/compositor_detect.hpp"
#include "i3space/fade.hpp"

#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <thread>

namespace i3space {

namespace {

constexpr unsigned long kBgColor = 0x2e2e2e;
constexpr unsigned long kCardBg = 0x3d3d3d;
constexpr unsigned long kCardSel = 0x005818;
constexpr unsigned long kBorder = 0x888888;
constexpr unsigned long kText = 0xffffff;

void set_wm_class(Display* dpy, Window win) {
  XClassHint hint{};
  hint.res_name = const_cast<char*>("i3space");
  hint.res_class = const_cast<char*>("i3space");
  XSetClassHint(dpy, win, &hint);
}

void set_window_type_notification(Display* dpy, Window win) {
  Atom net_wm_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
  Atom notification = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
  XChangeProperty(dpy, win, net_wm_type, XA_ATOM, 32, PropModeReplace,
                  reinterpret_cast<unsigned char*>(&notification), 1);
}

void set_opacity(Display* dpy, Window win, double opacity) {
  unsigned long card = static_cast<unsigned long>(opacity * 0xffffffffUL);
  Atom opacity_atom = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
  XChangeProperty(dpy, win, opacity_atom, XA_CARDINAL, 32, PropModeReplace,
                  reinterpret_cast<unsigned char*>(&card), 1);
}

}  // namespace

StripSession::StripSession(const Config& cfg) : cfg_(cfg) {}

StripSession::~StripSession() {
  close();
}

bool StripSession::setup_display() {
  display_ = XOpenDisplay(nullptr);
  if (!display_) {
    return false;
  }
  screen_ = DefaultScreen(display_);
  visual_ = DefaultVisual(display_, screen_);
  cmap_ = DefaultColormap(display_, screen_);
  depth_ = DefaultDepth(display_, screen_);
  return true;
}

void StripSession::build_strips(const WorkspaceView& view) {
  strips_.clear();
  for (const auto& og : view.outputs) {
    if (og.ws_indices.size() < 2) {
      continue;
    }
    OutputStrip strip;
    strip.output_name = og.name;
    strip.output_x = og.x;
    strip.output_y = og.y;
    strip.output_w = og.width > 0 ? og.width : DisplayWidth(display_, screen_);
    strip.output_h = og.height > 0 ? og.height : DisplayHeight(display_, screen_);

    const int card_w = cfg_.config.card_width;
    const int card_h = cfg_.config.card_height;
    const int margin = cfg_.config.strip_margin_px;
    strip.band_h = card_h + margin * 2;
    strip.band_y = strip.output_y + cfg_.config.bar_margin_px;

    int x = margin;
    for (int ws_idx : og.ws_indices) {
      CardLayout cl;
      cl.ws_index = ws_idx;
      cl.x = x;
      cl.y = margin;
      cl.w = card_w;
      cl.h = card_h;
      strip.cards.push_back(cl);
      x += card_w + margin;
    }
    strips_.push_back(std::move(strip));
  }
}

void StripSession::apply_opacity(Window win) {
  double opacity = cfg_.config.opacity;
  if (compositor_running()) {
    opacity = cfg_.transparency.opacity > 0 ? cfg_.transparency.opacity : opacity;
  }
  if (opacity < 1.0) {
    set_opacity(display_, win, opacity);
  }
}

void StripSession::draw_strip(size_t output_index, OutputStrip& strip,
                              const WorkspaceView& view) {
  const int card_w = cfg_.config.card_width;
  const int card_h = cfg_.config.card_height;

  if (strip.pixmap) {
    XFreePixmap(display_, strip.pixmap);
  }
  strip.pixmap = XCreatePixmap(display_, strip.window, strip.output_w, strip.band_h, depth_);

  GC gc = XCreateGC(display_, strip.pixmap, 0, nullptr);
  XSetForeground(display_, gc, kBgColor);
  XFillRectangle(display_, strip.pixmap, gc, 0, 0, strip.output_w, strip.band_h);

  XftDraw* xft = XftDrawCreate(display_, strip.pixmap, visual_, cmap_);
  XftColor xft_color{};
  XRenderColor rc{0xffff, 0xffff, 0xffff, 0xffff};
  XftColorAllocValue(display_, visual_, cmap_, &rc, &xft_color);
  XftFont* font = XftFontOpenName(display_, screen_, "sans-9");

  for (size_t i = 0; i < strip.cards.size(); ++i) {
    const auto& cl = strip.cards[i];
    const int draw_x = cl.x - strip.scroll_x;
    if (draw_x + cl.w < 0 || draw_x > strip.output_w) {
      continue;
    }
    const int gi = local_to_global(static_cast<int>(output_index), static_cast<int>(i));
    const bool is_sel = (gi == selected_card_);

    XSetForeground(display_, gc, is_sel ? kCardSel : kCardBg);
    XFillRectangle(display_, strip.pixmap, gc, draw_x, cl.y, cl.w, cl.h);
    XSetForeground(display_, gc, kBorder);
    XDrawRectangle(display_, strip.pixmap, gc, draw_x, cl.y, cl.w - 1, cl.h - 1);

    if (cl.ws_index >= 0 && cl.ws_index < static_cast<int>(view.workspaces.size())) {
      const auto& ws = view.workspaces[static_cast<size_t>(cl.ws_index)];
      if (font) {
        std::string label = ws.name;
        XftDrawStringUtf8(xft, &xft_color, font, draw_x + 6, cl.y + 16,
                          reinterpret_cast<const FcChar8*>(label.c_str()), label.size());
      }
      if (ws.first_win >= 0) {
        for (int w = 0; w < ws.win_count; ++w) {
          const auto& win = view.windows[static_cast<size_t>(ws.first_win + w)];
          const int rx = draw_x + 6 + static_cast<int>(win.norm_x * (cl.w - 12));
          const int ry = cl.y + 24 + static_cast<int>(win.norm_y * (cl.h - 32));
          const int rw = std::max(2, static_cast<int>(win.norm_w * (cl.w - 12)));
          const int rh = std::max(2, static_cast<int>(win.norm_h * (cl.h - 32)));
          XSetForeground(display_, gc, win.focused ? kText : kBorder);
          XFillRectangle(display_, strip.pixmap, gc, rx, ry, rw, rh);
        }
      }
    }
  }

  if (font) {
    XftFontClose(display_, font);
  }
  XftColorFree(display_, visual_, cmap_, &xft_color);
  XftDrawDestroy(xft);
  XFreeGC(display_, gc);

  GC wgc = XCreateGC(display_, strip.window, 0, nullptr);
  XCopyArea(display_, strip.pixmap, strip.window, wgc, 0, 0, strip.output_w, strip.band_h, 0, 0);
  XFreeGC(display_, wgc);
}

void StripSession::redraw_all(const WorkspaceView& view) {
  for (size_t oi = 0; oi < strips_.size(); ++oi) {
    draw_strip(oi, strips_[oi], view);
  }
}

int StripSession::local_to_global(int output, int local) const {
  int g = 0;
  for (int o = 0; o < output; ++o) {
    g += static_cast<int>(strips_[static_cast<size_t>(o)].cards.size());
  }
  return g + local;
}

int StripSession::global_to_local(int global, int& out_output, int& out_local) const {
  int g = 0;
  for (size_t o = 0; o < strips_.size(); ++o) {
    const int n = static_cast<int>(strips_[o].cards.size());
    if (global < g + n) {
      out_output = static_cast<int>(o);
      out_local = global - g;
      return 0;
    }
    g += n;
  }
  return -1;
}

int StripSession::card_count() const {
  int n = 0;
  for (const auto& s : strips_) {
    n += static_cast<int>(s.cards.size());
  }
  return n;
}

void StripSession::ensure_card_visible(OutputStrip& strip, int local_index) {
  if (local_index < 0 || local_index >= static_cast<int>(strip.cards.size())) {
    return;
  }
  const auto& cl = strip.cards[static_cast<size_t>(local_index)];
  const int margin = cfg_.config.strip_margin_px;
  const int visible_w = strip.output_w;
  if (cl.x - strip.scroll_x < margin) {
    strip.scroll_x = std::max(0, cl.x - margin);
  } else if (cl.x + cl.w - strip.scroll_x > visible_w - margin) {
    strip.scroll_x = std::max(0, cl.x + cl.w - visible_w + margin);
  }
}

bool StripSession::select_card(int global_index) {
  if (global_index < 0 || global_index >= card_count()) {
    return false;
  }
  selected_card_ = global_index;
  int out = 0;
  int loc = 0;
  global_to_local(global_index, out, loc);
  active_output_ = out;
  ensure_card_visible(strips_[static_cast<size_t>(out)], loc);
  return true;
}

bool StripSession::open(const WorkspaceView& view) {
  if (!setup_display()) {
    return false;
  }
  build_strips(view);
  if (strips_.empty()) {
    return false;
  }

  const int fade_ms = compositor_running() ? cfg_.transparency.fade_ms : 0;
  if (fade_ms > 0) {
    fade_delay_ms(fade_ms / 4);
  }

  for (auto& strip : strips_) {
    strip.window = XCreateSimpleWindow(
        display_, DefaultRootWindow(display_), strip.output_x, strip.band_y,
        static_cast<unsigned>(strip.output_w), static_cast<unsigned>(strip.band_h),
        0, kBorder, kBgColor);
    XSetWindowAttributes attrs{};
    attrs.override_redirect = True;
    attrs.colormap = cmap_;
    XChangeWindowAttributes(display_, strip.window, CWOverrideRedirect | CWColormap, &attrs);
    set_wm_class(display_, strip.window);
    set_window_type_notification(display_, strip.window);
    apply_opacity(strip.window);
    XSelectInput(display_, strip.window,
                 ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
    XMapRaised(display_, strip.window);
  }

  XSync(display_, False);
  select_card(0);
  redraw_all(view);

  if (strips_.size() == 1) {
    XGrabKeyboard(display_, strips_[0].window, True, GrabModeAsync, GrabModeAsync,
                  CurrentTime);
  } else {
    XGrabKeyboard(display_, strips_[static_cast<size_t>(active_output_)].window, True,
                  GrabModeAsync, GrabModeAsync, CurrentTime);
  }
  return true;
}

void StripSession::close() {
  if (!display_) {
    return;
  }
  XUngrabKeyboard(display_, CurrentTime);
  for (auto& strip : strips_) {
    if (strip.pixmap) {
      XFreePixmap(display_, strip.pixmap);
      strip.pixmap = 0;
    }
    if (strip.window) {
      XDestroyWindow(display_, strip.window);
      strip.window = 0;
    }
  }
  strips_.clear();
  XCloseDisplay(display_);
  display_ = nullptr;
}

int StripSession::run_loop(const WorkspaceView& view, std::string& selected_ws_out) {
  if (!display_) {
    return 1;
  }

  while (true) {
    XEvent ev;
    XNextEvent(display_, &ev);

    if (ev.type == Expose) {
      redraw_all(view);
      continue;
    }

    if (ev.type != KeyPress && ev.type != ButtonPress) {
      continue;
    }

    if (ev.type == ButtonPress) {
      for (size_t oi = 0; oi < strips_.size(); ++oi) {
        auto& strip = strips_[oi];
        if (ev.xany.window != strip.window) {
          continue;
        }
        const int mx = ev.xbutton.x + strip.scroll_x;
        const int my = ev.xbutton.y;
        for (size_t ci = 0; ci < strip.cards.size(); ++ci) {
          const auto& cl = strip.cards[ci];
          if (mx >= cl.x && mx < cl.x + cl.w && my >= cl.y && my < cl.y + cl.h) {
            const int gi = local_to_global(static_cast<int>(oi), static_cast<int>(ci));
            select_card(gi);
            redraw_all(view);
            const int ws_idx = strip.cards[ci].ws_index;
            if (ws_idx >= 0 && ws_idx < static_cast<int>(view.workspaces.size())) {
              selected_ws_out = view.workspaces[static_cast<size_t>(ws_idx)].name;
              return 0;
            }
          }
        }
      }
      continue;
    }

    KeySym sym = XLookupKeysym(&ev.xkey, 0);
    if (sym == XK_Escape) {
      return 2;
    }
    if (sym == XK_Return || sym == XK_KP_Enter) {
      int out = 0;
      int loc = 0;
      if (global_to_local(selected_card_, out, loc) == 0) {
        const int ws_idx = strips_[static_cast<size_t>(out)].cards[static_cast<size_t>(loc)].ws_index;
        if (ws_idx >= 0 && ws_idx < static_cast<int>(view.workspaces.size())) {
          selected_ws_out = view.workspaces[static_cast<size_t>(ws_idx)].name;
          return 0;
        }
      }
      return 2;
    }
    if (sym == XK_Left || sym == XK_h) {
      select_card((selected_card_ - 1 + card_count()) % card_count());
      redraw_all(view);
    } else if (sym == XK_Right || sym == XK_l) {
      select_card((selected_card_ + 1) % card_count());
      redraw_all(view);
    } else if (sym >= XK_1 && sym <= XK_6) {
      const int pos = static_cast<int>(sym - XK_1);
      if (pos < card_count()) {
        select_card(pos);
        redraw_all(view);
      }
    }
  }
}

int show_single_ws_toast(const Config& cfg, const OutputGroup& output, Display* dpy) {
  Display* display = dpy;
  bool own_display = false;
  if (!display) {
    display = XOpenDisplay(nullptr);
    own_display = true;
  }
  if (!display) {
    return 1;
  }

  const int screen = DefaultScreen(display);
  const int w = cfg.config.card_width;
  const int h = cfg.config.card_height;
  const int x = output.x + (output.width - w) / 2;
  const int y = output.y + cfg.config.bar_margin_px + cfg.config.strip_margin_px;

  Window win = XCreateSimpleWindow(display, DefaultRootWindow(display), x, y,
                                   static_cast<unsigned>(w), static_cast<unsigned>(h), 1,
                                   kBorder, kCardBg);
  XSetWindowAttributes attrs{};
  attrs.override_redirect = True;
  XChangeWindowAttributes(display, win, CWOverrideRedirect, &attrs);
  set_wm_class(display, win);
  XMapRaised(display, win);

  GC gc = XCreateGC(display, win, 0, nullptr);
  XSetForeground(display, gc, kText);
  const char* label = "X";
  if (!cfg.config.single_ws_message.empty()) {
    label = cfg.config.single_ws_message.c_str();
  }
  XDrawString(display, win, gc, w / 2 - 4, h / 2 + 4, label, static_cast<int>(strlen(label)));
  XFreeGC(display, gc);
  XSync(display, False);

  const int ms = cfg.config.single_ws_toast_ms > 0 ? cfg.config.single_ws_toast_ms : 2000;
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));

  XDestroyWindow(display, win);
  XSync(display, False);
  if (own_display) {
    XCloseDisplay(display);
  }
  return 0;
}

}  // namespace i3space
