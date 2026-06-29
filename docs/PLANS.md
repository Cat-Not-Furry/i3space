# i3space — execution chain

Standalone repo (not [destinmoulton/i3space](https://github.com/destinmoulton/i3space)).

## Dependency graph

```
Plan01 → Plan02 → Plan03 → Plan04 → Plan05 → Plan06
  → Plan07 → Plan08 → Plan09 → Plan10 → Plan11
```

## Metrics

| Metric | MVP1 target | MVP2 target |
|--------|-------------|-------------|
| IPC parse+layout p95 | < 5 ms | < 3 ms |
| Peak RAM (overview open) | < 8 MiB | < 10 MiB |
| Idle processes | 0 | 0 |
| Toggle to first frame | < 12 ms | < 8 ms |

## Config load order

1. `~/.config/i3space/i3space` or `i3space.conf`
2. Legacy split: `~/.config/i3/i3space.conf` + `~/.config/picom/i3space.conf`
3. Embedded defaults

Sections in unified file: `#!config`, `#!transparency`.

## Definition of done

### Plan 01 — Preparation
- [x] CMake builds stub/binary
- [x] `build-and-install.sh` (no Arch), `packaging/PKGBUILD` + `makepkg -si` (Arch)

### Plan 02 — Planning
- [x] This file with DoD and metrics

### Plan 03 — MVP1-A (core)
- [x] `i3_ipc.cpp` — GET_WORKSPACES, GET_TREE, GET_OUTPUTS, RUN_COMMAND
- [x] `tree_flatten.cpp` — simdjson SoA
- [x] `config.cpp`, `first_run.cpp`, `locale.cpp`
- [x] `i3space --print-active [--output NAME]`

Verify: `i3space --print-active` with i3 running.

### Plan 04 — MVP1-B (UI)
- [x] Top strip per output, 255×180 horizontal cards
- [x] Keyboard: arrows, 1–6, Enter, Esc
- [x] `i3space --toggle`, single-workspace toast
- [x] Overlay bar (`bar_margin_px=0` default)

Verify: bind `bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle`

### Plan 05 — Tests MVP1
- [x] `tests/bench_parse.sh`
- [x] `tests/check_version.sh`
- Manual: multi-monitor, 1 ws toast, first-run TTY

### Plan 06 — Retro MVP1
- [x] `docs/RETRO_MVP1.md`

### Plan 07 — MVP2 compositor
- [x] `compositor_detect.cpp`
- [x] `examples/picom/i3space.conf`
- [x] `_NET_WM_WINDOW_OPACITY` fallback

### Plan 08 — MVP2 input/FX
- [x] Mouse click on cards
- [x] `fade_ms` in config (optional delay)
- [x] Horizontal scroll via `scroll_step_px` + card navigation

### Plan 09 — Performance MVP2
- [x] `docs/PERF.md`, `tests/bench_toggle.sh`

### Plan 10 — Release docs
- [x] README, CHANGELOG, `man/i3space.1`
- [x] Link from configurations README

### Plan 11 — Release ship
- [x] `.github/workflows/build.yml`
- [x] `packaging/PKGBUILD.example`
- [ ] Git tag `v0.2.0` (run locally: `git tag -a v0.2.0 -m "i3space 0.2.0"`)

## i3 integration checklist

- [ ] `bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle`
- [ ] Optional picom: `include` examples/picom/i3space.conf
- [ ] Do **not** add bind to configurations `I3/i3-wm/config` by default
