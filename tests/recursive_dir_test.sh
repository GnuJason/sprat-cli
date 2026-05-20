#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: recursive_dir_test.sh <spratlayout-bin>" >&2
    exit 1
fi

spratlayout_bin="$1"

tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

# Path conversion for Windows
if [[ "$(uname)" == MINGW* || "$(uname)" == MSYS* ]]; then
    tmp_dir_win="$(cygpath -m "$tmp_dir")"
    fix_path() {
        echo "${1/$tmp_dir/$tmp_dir_win}"
    }
else
    fix_path() {
        echo "$1"
    }
fi

mkdir -p "$tmp_dir/frames/subdir"

create_png() {
    local path="$1"
    local w="$2"
    local h="$3"
    # Create a minimal valid PNG using printf if ImageMagick is not available
    # Or just use spratconvert if we had it, but we want to be independent if possible.
    # Actually, the repo has a lot of PNGs. Let's just copy one if it exists.
    local source_png
    source_png=$(find . -name "*.png" | head -n 1)
    if [ -n "$source_png" ]; then
        cp "$source_png" "$path"
    elif command -v magick >/dev/null; then
        magick -size "${w}x${h}" xc:red "$path"
    elif command -v convert >/dev/null; then
        convert -size "${w}x${h}" xc:red "$path"
    else
        echo "Error: ImageMagick not found and no source PNG found in repo" >&2
        exit 1
    fi
}

create_png "$tmp_dir/frames/top.png" 10 10
create_png "$tmp_dir/frames/subdir/nested.png" 10 10

# Helper to extract paths from spratlayout output
extract_sprite_paths() {
    grep "^sprite " | sed -E 's/^sprite ("[^"]*").*$/\1/'
}

echo "Running recursive directory test..."
OUTPUT=$("$spratlayout_bin" "$(fix_path "$tmp_dir/frames")")

if ! echo "$OUTPUT" | grep -q "nested.png"; then
    echo "FAILED: Did not find nested.png in subdirectory"
    echo "Output was:"
    echo "$OUTPUT"
    exit 1
fi

if ! echo "$OUTPUT" | grep -q "top.png"; then
    echo "FAILED: Did not find top.png in top-level directory"
    echo "Output was:"
    echo "$OUTPUT"
    exit 1
fi

echo "Recursive directory test passed!"
