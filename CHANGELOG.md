# Changelog

## 0.2.1

- Fix hang on `--print-active` / `--toggle`: **IPC header struct had 2-byte padding** (16 B sent instead of 14 B)
- Parse i3 tree with simdjson DOM (safe recursion vs ondemand)
- IPC: fresh socket per request, 10 s read timeout, ignore empty `I3SOCK`
- Clearer IPC error messages; flush JSON output early
- License aligned to GPL-3.0-or-later (was incorrectly labeled MIT in README/packaging)

## 0.2.0

- Compositor detection (picom, compton, xcompmgr)
- Mouse click on workspace cards
- Optional `fade_ms` and horizontal scroll for overflowing strips
- Picom example config
- Performance docs and benchmarks

## 0.1.0

- i3 IPC core and `--print-active`
- Config INI with `#!config` / `#!transparency`
- First-run setup (`~/.config/i3space/`)
- `--toggle` top strip per output
- Keyboard navigation and single-workspace toast
