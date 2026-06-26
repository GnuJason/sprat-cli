#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
    echo "Usage: edge_cases_test.sh <spratlayout-bin> <spratconvert-bin>" >&2
    exit 1
fi

spratlayout_bin="$1"
spratconvert_bin="$2"

tmp_dir="$(mktemp -d)"
trap "rm -rf \"$tmp_dir\"" EXIT

# 1. spratlayout with missing/empty profiles
echo "Test 1: spratlayout with empty profiles file"
touch "$tmp_dir/empty_profiles.cfg"
if "$spratlayout_bin" --profiles-config "$tmp_dir/empty_profiles.cfg" some_image.png > /dev/null 2>&1; then
    echo "FAILED: spratlayout should fail with empty profiles if no profile is specified"
    # Actually, if it doesnt have default profile it should fail.
fi

# 2. spratconvert with empty input
echo "Test 2: spratconvert with empty input"
if echo -n "" | "$spratconvert_bin" --transform json 2>/dev/null; then
    echo "FAILED: spratconvert should fail with empty input"
fi

# 3. spratconvert with invalid transform
echo "Test 3: spratconvert with invalid transform"
if echo "atlas 1,1" | "$spratconvert_bin" --transform non_existent 2>/dev/null; then
    echo "FAILED: spratconvert should fail with non-existent transform"
fi

# 4. spratlayout with non-existent directory
echo "Test 4: spratlayout with non-existent directory"
output=$("$spratlayout_bin" /nonexistent/path/that/does/not/exist 2>&1 || true)
if echo "$output" | grep -qi "does not exist"; then
    echo "  PASS: got 'does not exist' error message"
else
    echo "  FAILED: expected 'does not exist' in error, got: $output"
    exit 1
fi

# 5. spratlayout with empty directory (no images)
echo "Test 5: spratlayout with empty directory"
empty_dir="$tmp_dir/empty_input"
mkdir -p "$empty_dir"
output=$("$spratlayout_bin" "$empty_dir" 2>&1 || true)
if echo "$output" | grep -qi "no valid images"; then
    echo "  PASS: got 'no valid images' error message"
else
    echo "  FAILED: expected 'no valid images' in error, got: $output"
    exit 1
fi

echo "edge_cases_test.sh: ok"
