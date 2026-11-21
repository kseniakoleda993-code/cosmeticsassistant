#!/bin/sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"

case "${1:-}" in
    "")
        cmake -S . -B build-tests
        cmake --build build-tests --parallel "${JOBS:-4}"
        ctest --test-dir build-tests --output-on-failure --parallel "${JOBS:-4}"
        ;;
    --coverage)
        build=build-coverage
        profile=$(mktemp -d)
        trap 'rm -f -- "$profile"/*.profraw "$profile/merged.profdata"; rmdir -- "$profile"' EXIT
        command -v llvm-profdata >/dev/null 2>&1 || command -v xcrun >/dev/null
        command -v llvm-cov >/dev/null 2>&1 || command -v xcrun >/dev/null
        llvm() {
            tool=$1
            shift
            if command -v "$tool" >/dev/null 2>&1; then
                "$tool" "$@"
            else
                xcrun "$tool" "$@"
            fi
        }
        cmake -S . -B "$build" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="${CXX:-clang++}" -DCOSMETICS_COVERAGE=ON
        cmake --build "$build" --parallel "${JOBS:-4}"
        LLVM_PROFILE_FILE="$profile/%p.profraw" ctest --test-dir "$build" --output-on-failure --parallel "${JOBS:-4}"
        llvm llvm-profdata merge -sparse "$profile"/*.profraw -o "$profile/merged.profdata"
        llvm llvm-cov report "$build/cosmetics_bot" -object "$build/cosmetics_tests" -instr-profile="$profile/merged.profdata" -ignore-filename-regex='(/tests/|/_deps/|/usr/)'
        ;;
    *)
        echo "Использование: scripts/test.sh [--coverage]" >&2
        exit 2
        ;;
esac
