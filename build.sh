#!/usr/bin/env bash
# Author: Gino Francesco Bogo

set -euo pipefail

clear

BUILD_TYPE=${1:-Debug}

echo "Build type: $BUILD_TYPE"

if [ ! -d "build" ]; then
    mkdir build
fi

cd ./build

cmake "../CMakeLists.txt" -B ./ -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build ./ --config $BUILD_TYPE

# Only copy compile_commands.json if it doesn't exist in parent directory or is different
if [ -f "compile_commands.json" ]; then
    if [ ! -f "../compile_commands.json" ] || ! cmp -s "compile_commands.json" "../compile_commands.json"; then
        cp "compile_commands.json" "../"
    fi
fi

echo '... Done!'
