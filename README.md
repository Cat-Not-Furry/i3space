# i3space

[English](#english) · [Español](#español)

---

## English

Keyboard-driven workspace overview for **i3** on X11. Inspired by Hyprspace; config-agnostic (only active workspaces via IPC).

**Not related to** [destinmoulton/i3space](https://github.com/destinmoulton/i3space) (unrelated Go/Fyne blank window).

### Requirements

- i3 (running)
- X11 session
- Build: CMake 3.20+, C++20, libX11, libXft, libXrender, fontconfig, simdjson
- Optional: picom/compton for blur (see `examples/picom/i3space.conf`)

### Build

#### Arch Linux

```bash
cd packaging
./prepare-sources.sh   # if i3space-<version>.tar.gz is missing
makepkg -si
```

See [packaging/README.md](packaging/README.md).

#### Other distros

```bash
./build-and-install.sh
# or
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

System-wide install (non-Arch): `./build-and-install.sh --system`

### First run

Config directory: **`~/.config/i3space/`**

- Interactive welcome if stdin is a TTY (en/es via `LANG`)
- Otherwise writes default config silently

### Usage

```bash
i3space --print-active          # JSON by output
i3space --print-active --output eDP-1
i3space --toggle                # open overview
```

#### i3 bind (document only — add to your config)

```i3
bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle
```

Use `--release` so keyboard grab works ([i3 userguide](https://i3wm.org/docs/userguide.html)).

### Config

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

### Behaviour

- One horizontal strip **per monitor** (top, overlays bar by default)
- Only workspaces that **exist** in i3 (IPC)
- Keys: arrows, `1`–`6` (list position), Enter, Esc
- One workspace on output → ✕ toast ~2s (no overview)
- Mouse: click card to switch (v0.2)

### Tests

```bash
chmod +x tests/*.sh
./tests/check_version.sh i3space          # after makepkg -si
./tests/test_flatten.sh i3space
./tests/bench_parse.sh i3space 20
```

Or with a local build: `./tests/check_version.sh ./build/i3space`

### Roadmap

See [docs/PLANS.md](docs/PLANS.md).

### License

**GPL-3.0-or-later** — see [LICENSE](LICENSE) (GNU General Public License v3).

i3space — Copyright (C) 2026 Cat-Not-Furry

---

## Español

Overview de workspaces para **i3** en X11, controlado por teclado. Inspirado en Hyprspace; agnóstico a la config (solo workspaces activos vía IPC).

**No relacionado con** [destinmoulton/i3space](https://github.com/destinmoulton/i3space) (proyecto distinto en Go/Fyne).

### Requisitos

- i3 en ejecución
- Sesión X11
- Compilación: CMake 3.20+, C++20, libX11, libXft, libXrender, fontconfig, simdjson
- Opcional: picom/compton para blur (ver `examples/picom/i3space.conf`)

### Compilación

#### Arch Linux

```bash
cd packaging
./prepare-sources.sh   # si falta i3space-<versión>.tar.gz
makepkg -si
```

Ver [packaging/README.md](packaging/README.md).

#### Otras distros

```bash
./build-and-install.sh
# o
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Instalación en todo el sistema (no Arch): `./build-and-install.sh --system`

### Primera ejecución

Directorio de config: **`~/.config/i3space/`**

- Bienvenida interactiva si stdin es una TTY (en/es según `LANG`)
- Si no, escribe la config por defecto en silencio

### Uso

```bash
i3space --print-active          # JSON por output
i3space --print-active --output eDP-1
i3space --toggle                # abre el overview
```

#### Atajo en i3 (solo documentación — añade a tu config)

```i3
bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle
```

Usa `--release` para que el grab de teclado funcione ([guía de i3](https://i3wm.org/docs/userguide.html)).

### Configuración

Archivo unificado `~/.config/i3space/i3space`:

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

Ver `examples/i3space.default` y `examples/picom/i3space.conf`.

### Comportamiento

- Una banda horizontal **por monitor** (arriba; superpone la barra por defecto)
- Solo workspaces que **existen** en i3 (IPC)
- Teclas: flechas, `1`–`6` (posición en la lista), Enter, Esc
- Un solo workspace en el output → toast ✕ ~2 s (sin overview)
- Ratón: clic en tarjeta para cambiar (v0.2)

### Pruebas

```bash
chmod +x tests/*.sh
./tests/check_version.sh i3space          # tras makepkg -si
./tests/test_flatten.sh i3space
./tests/bench_parse.sh i3space 20
```

O con binario local: `./tests/check_version.sh ./build/i3space`

### Roadmap

Ver [docs/PLANS.md](docs/PLANS.md).

### Licencia

**GPL-3.0-or-later** — ver [LICENSE](LICENSE) (GNU General Public License v3).

i3space — Copyright (C) 2026 Cat-Not-Furry
