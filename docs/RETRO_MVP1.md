# Retro MVP1 — i3space

## Delivered (v0.1.0 scope)

- i3 IPC: workspaces, tree, outputs, run_command
- Config-agnostic active workspace list (`--print-active`)
- INI config with `#!config` / `#!transparency` in `~/.config/i3space/`
- First-run wizard (TTY) or silent default config
- Top horizontal strip per output (255×180 cards)
- Keyboard navigation; mouse click (MVP2 scope, implemented early)
- Single-workspace toast (✕ or `single_ws_message`)
- `bar_margin_px` default 0 (overlays bar like dmenu)

## Gaps vs Hyprspace

- No live window thumbnails (by design)
- No fullscreen dim overlay (horizontal band only)
- Fade minimal (`fade_ms` optional)
- Compositor blur via picom rules only (documented, not auto-patched)

## Bugs / follow-ups

- Test on real multi-monitor i3 session
- Workspace names with special characters in `run_command`
- Xft font fallback if `sans-9` missing

## MVP2 scope (locked)

- Compositor detection (picom/compton/xcompmgr)
- `examples/picom/i3space.conf`
- Horizontal scroll when cards overflow
- Optional fade on open
- Performance targets in `docs/PERF.md`
