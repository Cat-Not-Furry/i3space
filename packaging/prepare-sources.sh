#!/usr/bin/env bash
# Genera i3space-<pkgver>.tar.gz y actualiza sha256sums en PKGBUILD + .SRCINFO.
set -euo pipefail

PACKAGING_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$PACKAGING_DIR/.." && pwd)"
PKGREL="${PKGREL:-1}"

read_pkgver() {
  grep -E '^project\(i3space VERSION ' "$ROOT/CMakeLists.txt" \
    | sed -E 's/.*VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/'
}

pkgver="$(read_pkgver)"
tarball="$PACKAGING_DIR/i3space-${pkgver}.tar.gz"
pkgbuild="$PACKAGING_DIR/PKGBUILD"

if [[ ! -f "$pkgbuild" ]]; then
  echo "Falta $pkgbuild" >&2
  exit 1
fi

echo "==> pkgver $pkgver"
rm -f "$tarball"

if git -C "$ROOT" rev-parse HEAD >/dev/null 2>&1; then
  if ! git -C "$ROOT" diff-index --quiet HEAD --; then
    echo "ERROR: hay cambios sin commitear en el repo." >&2
    echo "git archive solo empaqueta HEAD; commitea o descarta antes de continuar." >&2
    exit 1
  fi
  git -C "$ROOT" archive --format=tar.gz \
    --prefix="i3space-${pkgver}/" \
    -o "$tarball" HEAD
else
  echo "No es un repo git; crea el tarball manualmente en $tarball" >&2
  exit 1
fi

sha256="$(sha256sum "$tarball" | awk '{print $1}')"
echo "==> $tarball"
echo "==> sha256 $sha256"

if grep -q '^sha256sums=' "$pkgbuild"; then
  sed -i "s/^sha256sums=.*/sha256sums=('${sha256}')/" "$pkgbuild"
else
  echo "sha256sums=('${sha256}')" >>"$pkgbuild"
fi

if grep -q '^pkgver=' "$pkgbuild"; then
  sed -i "s/^pkgver=.*/pkgver=${pkgver}/" "$pkgbuild"
fi
if grep -q '^pkgrel=' "$pkgbuild"; then
  sed -i "s/^pkgrel=.*/pkgrel=${PKGREL}/" "$pkgbuild"
fi

if command -v makepkg >/dev/null 2>&1; then
  if (cd "$PACKAGING_DIR" && makepkg --printsrcinfo > .SRCINFO); then
    echo "==> .SRCINFO actualizado"
  else
    echo "makepkg --printsrcinfo falló; actualiza .SRCINFO a mano si hace falta" >&2
  fi
else
  echo "makepkg no encontrado; .SRCINFO no actualizado" >&2
fi

echo "Listo. Empaqueta e instala con: cd packaging && makepkg -si"
