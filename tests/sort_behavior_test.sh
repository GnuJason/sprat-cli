#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: sort_behavior_test.sh <spratlayout-bin>" >&2
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

# Create test images of different sizes to verify sorting
mkdir -p "$tmp_dir/frames"

create_png() {
    local path="$1"
    local w="$2"
    local h="$3"
    if command -v magick >/dev/null; then
        magick -size "${w}x${h}" xc:red "$path"
    elif command -v convert >/dev/null; then
        convert -size "${w}x${h}" xc:red "$path"
    else
        echo "Error: ImageMagick not found" >&2
        exit 1
    fi
}

create_png "$tmp_dir/frames/b.png" 40 40
create_png "$tmp_dir/frames/a.png" 10 10
create_png "$tmp_dir/frames/c.png" 20 20
create_png "$tmp_dir/frames/d.png" 30 30

# Natural sorting tests (used with --sort name)
create_png "$tmp_dir/frames/walk_2.png" 10 10
create_png "$tmp_dir/frames/walk_10.png" 10 10
create_png "$tmp_dir/frames/walk 2.png" 10 10
create_png "$tmp_dir/frames/walk 10.png" 10 10
create_png "$tmp_dir/frames/walk (2).png" 10 10
create_png "$tmp_dir/frames/walk (10).png" 10 10

# Helper to extract paths from spratlayout output
# Matches: sprite "path" x,y w,h ...
# Extracts: "path"
extract_sprite_paths() {
    grep "^sprite " | sed -E 's/^sprite ("[^"]*").*$/\1/'
}

# Test 1: Default behavior for directory (should NOT sort by name; optimization allowed)
echo "Test 1: Default behavior (no name sort, optimization allowed)"
"$spratlayout_bin" "$(fix_path "$tmp_dir/frames")" --mode fast | extract_sprite_paths | sed "s|$(fix_path "$tmp_dir/frames/")||g" | grep -E '^"[abcd]\.png"$' > "$tmp_dir/out_default.txt"
# In fast mode sprites are height-sorted: b(40) > d(30) > c(20) > a(10)
cat > "$tmp_dir/expected_default.txt" <<EOF
"b.png"
"d.png"
"c.png"
"a.png"
EOF

if ! diff -u "$tmp_dir/expected_default.txt" "$tmp_dir/out_default.txt"; then
    echo "FAILED: Default behavior should allow optimization (height sort in FAST mode), not sort by name"
    exit 1
fi

# Test 2: --sort none with list (should ALLOW optimization, e.g. height sort in FAST mode)
echo "Test 2: --sort none with list (should optimize)"
list_file="$tmp_dir/list_none.txt"
cat > "$list_file" <<EOF
$(fix_path "$tmp_dir/frames/c.png")
$(fix_path "$tmp_dir/frames/b.png")
$(fix_path "$tmp_dir/frames/d.png")
$(fix_path "$tmp_dir/frames/a.png")
EOF
"$spratlayout_bin" "$(fix_path "$list_file")" --mode fast --sort none | extract_sprite_paths | sed -e "s|$(fix_path "$tmp_dir/frames/")||g" -e "s|frames/||g" > "$tmp_dir/out_none.txt"
# Height order: b (40), d (30), c (20), a (10)
cat > "$tmp_dir/expected_none.txt" <<EOF
"b.png"
"d.png"
"c.png"
"a.png"
EOF

if ! diff -u "$tmp_dir/expected_none.txt" "$tmp_dir/out_none.txt"; then
    echo "FAILED: --sort none should allow optimization (height sort in FAST mode)"
    exit 1
fi

# Test 3: List input WITHOUT --sort (should be height sort by default in FAST mode)
echo "Test 3: List input without --sort (should optimize)"
list_file_default="$tmp_dir/list_default.txt"
cat > "$list_file_default" <<EOF
$(fix_path "$tmp_dir/frames/a.png")
$(fix_path "$tmp_dir/frames/b.png")
$(fix_path "$tmp_dir/frames/c.png")
$(fix_path "$tmp_dir/frames/d.png")
EOF
"$spratlayout_bin" "$(fix_path "$list_file_default")" --mode fast | extract_sprite_paths | sed -e "s|$(fix_path "$tmp_dir/frames/")||g" -e "s|frames/||g" > "$tmp_dir/out_list_default.txt"
# Height order: b (40), d (30), c (20), a (10)
cat > "$tmp_dir/expected_list_default.txt" <<EOF
"b.png"
"d.png"
"c.png"
"a.png"
EOF

if ! diff -u "$tmp_dir/expected_list_default.txt" "$tmp_dir/out_list_default.txt"; then
    echo "FAILED: List input without --sort should be height sort by default in FAST mode"
    exit 1
fi

# Test 4: --sort name with list (should enforce alphabetical order)
echo "Test 4: --sort name with list (should enforce name order)"
list_file_name="$tmp_dir/list_name.txt"
cat > "$list_file_name" <<EOF
$(fix_path "$tmp_dir/frames/c.png")
$(fix_path "$tmp_dir/frames/b.png")
$(fix_path "$tmp_dir/frames/d.png")
$(fix_path "$tmp_dir/frames/a.png")
EOF
"$spratlayout_bin" "$(fix_path "$list_file_name")" --mode fast --sort name | extract_sprite_paths | sed -e "s|$(fix_path "$tmp_dir/frames/")||g" -e "s|frames/||g" > "$tmp_dir/out_name.txt"
cat > "$tmp_dir/expected_name.txt" <<EOF
"a.png"
"b.png"
"c.png"
"d.png"
EOF

if ! diff -u "$tmp_dir/expected_name.txt" "$tmp_dir/out_name.txt"; then
    echo "FAILED: --sort name should enforce natural name order"
    exit 1
fi

# Test 5: --sort name with directory (should use natural number ordering)
echo "Test 5: --sort name with directory (natural number sort)"
"$spratlayout_bin" "$(fix_path "$tmp_dir/frames")" --mode fast --sort name | extract_sprite_paths | sed "s|$(fix_path "$tmp_dir/frames/")||g" | grep "walk" > "$tmp_dir/out_walk.txt"

cat > "$tmp_dir/expected_walk.txt" <<EOF
"walk (2).png"
"walk (10).png"
"walk 2.png"
"walk 10.png"
"walk_2.png"
"walk_10.png"
EOF

if ! diff -u "$tmp_dir/expected_walk.txt" "$tmp_dir/out_walk.txt"; then
    echo "FAILED: --sort name should use natural number ordering (2 before 10)"
    exit 1
fi

echo "All sort behavior tests passed!"
