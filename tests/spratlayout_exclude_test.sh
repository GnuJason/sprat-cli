#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: spratlayout_exclude_test.sh <spratlayout-bin>" >&2
    exit 1
fi

spratlayout_bin="$1"

tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

if command -v magick >/dev/null; then
    create_image_cmd="magick"
elif command -v convert >/dev/null; then
    create_image_cmd="convert"
else
    echo "Error: ImageMagick not found" >&2
    exit 1
fi

mkdir -p "$tmp_dir/frames"
"$create_image_cmd" -size 8x8 xc:red "$tmp_dir/frames/a.png"
"$create_image_cmd" -size 8x8 xc:green "$tmp_dir/frames/b.png"
"$create_image_cmd" -size 8x8 xc:blue "$tmp_dir/frames/c.png"

cat > "$tmp_dir/frames/.spratlayoutignore" <<'EOF'
# Persistently exclude this sprite from directory scans
exclude "b.png"
EOF

dir_layout="$tmp_dir/dir_layout.txt"
"$spratlayout_bin" "$tmp_dir/frames" > "$dir_layout"

if grep -q 'sprite "b\.png"' "$dir_layout"; then
    echo "FAILED: directory scan should honor .spratlayoutignore" >&2
    cat "$dir_layout" >&2
    exit 1
fi

if ! grep -q 'sprite "a\.png"' "$dir_layout" || ! grep -q 'sprite "c\.png"' "$dir_layout"; then
    echo "FAILED: directory scan omitted non-excluded sprites" >&2
    cat "$dir_layout" >&2
    exit 1
fi

cat > "$tmp_dir/frames.txt" <<'EOF'
root "frames"
exclude "c.png"
a.png
b.png
c.png
EOF

list_layout="$tmp_dir/list_layout.txt"
"$spratlayout_bin" "$tmp_dir/frames.txt" > "$list_layout"

if grep -q 'sprite "frames/c\.png"' "$list_layout"; then
    echo "FAILED: list input should honor exclude directive" >&2
    cat "$list_layout" >&2
    exit 1
fi

if ! grep -q 'sprite "frames/a\.png"' "$list_layout" || ! grep -q 'sprite "frames/b\.png"' "$list_layout"; then
    echo "FAILED: list input omitted non-excluded sprites" >&2
    cat "$list_layout" >&2
    exit 1
fi

echo "Spratlayout exclude test passed!"
