#!/usr/bin/env bash
# Build i3space from source (non-Arch distros). Arch: use packaging/PKGBUILD + makepkg -si.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
INSTALL_SYSTEM=0
PREFIX="${PREFIX:-$HOME/.local}"

usage() {
  cat <<EOF
Uso: $(basename "$0") [--system] [--prefix DIR]

Compila i3space con CMake e instala el binario (distros que no usan pacman/makepkg).

  --prefix DIR   Instalar en PREFIX/bin (default: ~/.local)
  --system       Instalar en /usr/local/bin (requiere sudo)
  -h, --help     Esta ayuda

Arch Linux:
  cd packaging && ./prepare-sources.sh && makepkg -si
  Ver packaging/README.md

Variables: BUILD_DIR (default: $ROOT/build)

Ejemplos:
  $(basename "$0")
  $(basename "$0") --system
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --system) INSTALL_SYSTEM=1; shift ;;
    --prefix) PREFIX="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Opción desconocida: $1" >&2; exit 1 ;;
  esac
done

if [[ -f /etc/arch-release ]]; then
  cat <<EOF >&2
Arch Linux detectado: este script es para otras distros.

Usa el empaquetado oficial:
  cd "$ROOT/packaging"
  ./prepare-sources.sh    # si falta i3space-<version>.tar.gz
  makepkg -si

Ver: packaging/README.md
EOF
  exit 1
fi

echo "==> cmake build"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc 2>/dev/null || echo 2)"
strip "$BUILD_DIR/i3space" 2>/dev/null || true

if (( INSTALL_SYSTEM )); then
  sudo install -m 755 "$BUILD_DIR/i3space" /usr/local/bin/i3space
  echo "Instalado: /usr/local/bin/i3space"
else
  install -d "$PREFIX/bin"
  install -m 755 "$BUILD_DIR/i3space" "$PREFIX/bin/i3space"
  echo "Instalado: $PREFIX/bin/i3space"
fi
