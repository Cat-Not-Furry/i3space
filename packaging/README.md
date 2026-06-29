# Arch Linux — i3space

En Arch el binario se instala con **makepkg** y **pacman**, no con `../build-and-install.sh`.

## Requisitos

```bash
sudo pacman -S --needed base-devel cmake simdjson i3-wm libxft libxrender fontconfig
```

(`makepkg -si` puede instalar `makedepends` con `-s`.)

## Instalar

```bash
cd packaging

# Solo la primera vez o tras cambiar versión en CMakeLists.txt:
./prepare-sources.sh

makepkg -si
```

- **`prepare-sources.sh`** — crea `i3space-<pkgver>.tar.gz`, actualiza `sha256sums` en `PKGBUILD` y regenera `.SRCINFO`. Requiere **working tree limpio** (`git archive` usa `HEAD`).
- **`makepkg -si`** — compila, empaqueta e instala con pacman.

Si falla CMake por caché de una versión anterior (`does not match the source`):

```bash
rm -rf src/build src/i3space-*/
makepkg -si
```

## AUR

Sube al repo del AUR:

- `PKGBUILD`
- `.SRCINFO`

Tras cada release: `./prepare-sources.sh`, commit, push. El tarball del AUR lo descargan los usuarios desde `source=` (o publicas el tag en GitHub y apuntas `source` a la URL del archive).

## Archivos generados (gitignore)

- `i3space-*.tar.gz`
- `src/`, `pkg/`
- `*.pkg.tar.zst`
