#!/usr/bin/env bash
# Build i3space and install locally or system-wide.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
INSTALL_SYSTEM=0
PREFIX="${PREFIX:-$HOME/.local}"

usage() {
  cat <<EOF
Uso: $(basename "$0") [--system] [--prefix DIR]

  --system   Install to /usr/local/bin (requires sudo)
  --prefix   Install to PREFIX/bin (default: ~/.local)
  -h, --help This help

Build dir: \$BUILD_DIR or $ROOT/build
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
