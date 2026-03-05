#!/bin/bash
# sync_lib.sh
# Copies Boring_Project/lib/ into every subdirectory that contains a .ino sketch.
# Run from anywhere inside the project — the script uses its own location as root.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_SRC="$SCRIPT_DIR/lib"

if [ ! -d "$LIB_SRC" ]; then
  echo "ERROR: $LIB_SRC does not exist." >&2
  exit 1
fi

found=0
for sketch_dir in "$SCRIPT_DIR"/*/; do
  [ -d "$sketch_dir" ] || continue

  # Only process directories that contain at least one .ino file
  shopt -s nullglob
  ino=("$sketch_dir"*.ino)
  shopt -u nullglob
  [ ${#ino[@]} -eq 0 ] && continue

  dest="${sketch_dir}lib"
  name=$(basename "$sketch_dir")

  # Remove existing lib (whether a symlink or a real directory)
  [ -L "$dest" ] && rm "$dest"
  [ -d "$dest" ] && rm -rf "$dest"

  cp -r "$LIB_SRC" "$dest"
  echo "Synced -> $name/lib"
  found=$((found + 1))
done

echo "$found sketch(es) updated."
