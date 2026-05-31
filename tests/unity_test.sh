#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "Usage: unity_test.sh <spratconvert-bin>" >&2
  exit 1
fi

convert_bin="$1"
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

layout_file="$tmp_dir/layout.txt"
cat > "$layout_file" <<'LAYOUT'
atlas 100,200
scale 1
sprite "player.png" 10,20 30,40 5,5 5,5
LAYOUT

markers_file="$tmp_dir/markers.txt"
cat > "$markers_file" <<'MARKERS'
path "player.png"
- marker "pivot" point 20,30
MARKERS

custom_transform="$tmp_dir/test.jsonnet"
cat > "$custom_transform" <<'CUSTOM'
local sprat = std.extVar("sprat");
local lib = import "sprat.libsonnet";
local sprite_line(s) =
  "y=" + s.y + " unity_y=" + s.unity_y +
  " pxn=" + lib.format_double(s.pivot_x_norm) +
  " pyn=" + lib.format_double(s.pivot_y_norm) +
  " pynr=" + lib.format_double(s.pivot_y_norm_raw) +
  " nh=" + s.name_hash_decimal +
  " nhh=" + s.name_hash_hex;
{
  name: "test",
  extension: "",
  content: std.join("\n", [sprite_line(s) for s in sprat.sprites]) + "\n",
}
CUSTOM

"$convert_bin" --transform "$custom_transform" --markers "$markers_file" < "$layout_file" > "$tmp_dir/out.txt"

grep -q "y=20" "$tmp_dir/out.txt"
grep -q "unity_y=140" "$tmp_dir/out.txt"
grep -q "pxn=0.5" "$tmp_dir/out.txt"
grep -q "pyn=0.4" "$tmp_dir/out.txt"
grep -q "pynr=0.6" "$tmp_dir/out.txt"
grep -q "nh=" "$tmp_dir/out.txt"
grep -q "nhh=" "$tmp_dir/out.txt"
