#!/bin/bash

# Exit if any command fails
set -e

# Derive workspace from THIS script's own location, regardless of where it's invoked from.
workspace="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${workspace}"

# vcpkg bootstrap (once per machine, gitignored)
# Dependencies are declared in vcpkg.json; cmake picks them up via the toolchain file below.
if [ ! -f "vcpkg/vcpkg" ]; then
  echo "Bootstrapping vcpkg..."
  if [ -d "vcpkg" ]; then
    echo "Removing incomplete vcpkg checkout..."
    rm -rf vcpkg
  fi
  git clone https://github.com/microsoft/vcpkg.git vcpkg
  ./vcpkg/bootstrap-vcpkg.sh -disableMetrics
fi

# Optional manual clean: ./run.sh --clean
if [ "${1:-}" = "--clean" ]; then
  echo "Removing build directory (--clean requested)..."
  rm -rf build
fi

# Configure (safe to re-run every time; CMake only regenerates what's stale)
# Local dev builds default to Debug for faster iteration; override with BUILD_TYPE=Release
build_type="${BUILD_TYPE:-Debug}"
echo "Configuring with CMake + Ninja (${build_type})..."
if ! cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE="${build_type}" \
    -DCMAKE_TOOLCHAIN_FILE="${workspace}/vcpkg/scripts/buildsystems/vcpkg.cmake"; then
  echo "CMake configure failed!"
  exit 1
fi

# Build (incremental: only rebuilds what changed since the last run)
echo "Building..."
if ! cmake --build build --parallel; then
  echo "Build failed!"
  exit 1
fi

echo -e "Build completed successfully! :) \n"

read -p "Do you want to run the game? (Y/n) " answer
case ${answer:0:1} in
    n|N )
        echo "Not running the game. You can run it later with './build/Othello'."
    ;;
    y|Y|"" )
        echo
        ./build/Othello
    ;;
esac