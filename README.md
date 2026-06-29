# i3space

Keyboard-driven workspace overview for **i3** on X11. Inspired by Hyprspace; config-agnostic (only active workspaces via IPC).

**Not related to** [destinmoulton/i3space](https://github.com/destinmoulton/i3space) (unrelated Go/Fyne blank window).

## Requirements

- i3 (running)
- X11 session
- Build: CMake 3.20+, C++20, libX11, libXft, libXrender, fontconfig, simdjson
- Optional: picom/compton for blur (see `examples/picom/i3space.conf`)

## Build

### Arch Linux

```bash
cd packaging
./prepare-sources.sh   # si falta i3space-<version>.tar.gz
makepkg -si
```

Ver [packaging/README.md](packaging/README.md).

### Otras distros

```bash
./build-and-install.sh
# o
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Instalación system-wide (no Arch): `./build-and-install.sh --system`

## First run

Config directory: **`~/.config/i3space/`**

- Interactive welcome if stdin is a TTY (en/es via `LANG`)
- Otherwise writes default config silently

## Usage

```bash
i3space --print-active          # JSON by output
i3space --print-active --output eDP-1
i3space --toggle                # open overview
```

### i3 bind (document only — add to your config)

```i3
bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle
```

Use `--release` so keyboard grab works ([i3 userguide](https://i3wm.org/docs/userguide.html)).

## Config

Unified file `~/.config/i3space/i3space`:

```ini
#!config
bar_margin_px = 0
card_width = 255
card_height = 180
single_ws_message =

#!transparency
opacity = 0.92
fade_ms = 0
```

See `examples/i3space.default` and `examples/picom/i3space.conf`.

## Behaviour

- One horizontal strip **per monitor** (top, overlays bar by default)
- Only workspaces that **exist** in i3 (IPC)
- Keys: arrows, `1`–`6` (list position), Enter, Esc
- One workspace on output → ✕ toast ~2s (no overview)
- Mouse: click card to switch (v0.2)

## Tests

```bash
chmod +x tests/*.sh
./tests/check_version.sh i3space          # tras makepkg -si
./tests/test_flatten.sh i3space
./tests/bench_parse.sh i3space 20
```

O con binario local: `./tests/check_version.sh ./build/i3space`

## Roadmap

See [docs/PLANS.md](docs/PLANS.md).

## License

**GPL-3.0-or-later** — see [LICENSE](LICENSE) (GNU General Public License v3).

i3space — Copyright (C) 2026 Cat-Not-Furry
