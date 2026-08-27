#!/usr/bin/env bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="$repo_root/out/build/c-prime-linux"
compiler="$build_dir/c-prime"
generated_dir="$build_dir/fixture-output"

if [[ ! -x "$compiler" ]]; then
    cmake --preset c-prime-linux >/dev/null || exit 1
fi

cmake --build "$build_dir" >/dev/null || exit 1
mkdir -p "$generated_dir"

failures=0

run_valid_fixture() {
    local fixture="$1"
    local name
    name="$(basename "$fixture" .cprime)"
    local cpp_output="$generated_dir/$name.cpp"
    local binary_output="$generated_dir/$name"

    if "$compiler" --emit-and-compile -o "$cpp_output" -b "$binary_output" "$fixture" >/dev/null; then
        if "$binary_output" >/dev/null; then
            echo "PASS valid   $name"
        else
            echo "FAIL valid   $name: generated binary failed"
            failures=$((failures + 1))
        fi
    else
        echo "FAIL valid   $name: transpile/compile failed"
        failures=$((failures + 1))
    fi
}

run_invalid_fixture() {
    local fixture="$1"
    local name
    name="$(basename "$fixture" .cprime)"
    local cpp_output="$generated_dir/$name.invalid.cpp"

    if "$compiler" --emit-cpp -o "$cpp_output" "$fixture" >/dev/null 2>&1; then
        echo "FAIL invalid $name: expected compiler failure"
        failures=$((failures + 1))
    else
        echo "PASS invalid $name"
    fi
}

for fixture in "$repo_root"/tests/fixtures/valid/*.cprime; do
    [[ -e "$fixture" ]] || continue
    run_valid_fixture "$fixture"
done

for fixture in "$repo_root"/tests/fixtures/invalid/*.cprime; do
    [[ -e "$fixture" ]] || continue
    run_invalid_fixture "$fixture"
done

if [[ "$failures" -ne 0 ]]; then
    echo "$failures fixture test(s) failed"
    exit 1
fi

echo "All fixture tests passed"