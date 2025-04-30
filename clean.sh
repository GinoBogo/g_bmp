#!/usr/bin/env bash
# Author: Gino Francesco Bogo

set -euo pipefail

clear

if [ -d "build" ]; then
    cmake --build ./build --target clean
fi

echo '... Done!'

