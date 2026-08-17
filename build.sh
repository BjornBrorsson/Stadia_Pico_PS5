#!/bin/bash
set -e

echo "=========================================================="
echo "  Stadia to PS5/Wingman Bridge - Raspberry Pi Pico WH Build"
echo "=========================================================="

BUILD_TYPE="${1:-Release}"

if [ -z "$PICO_SDK_PATH" ]; then
    if [ -d "$HOME/pico-sdk" ]; then
        export PICO_SDK_PATH="$HOME/pico-sdk"
        echo "Found Pico SDK at: $PICO_SDK_PATH"
    elif [ -d "$HOME/.pico-sdk/sdk/2.0.0" ]; then
        export PICO_SDK_PATH="$HOME/.pico-sdk/sdk/2.0.0"
        echo "Found Pico SDK at: $PICO_SDK_PATH"
    else
        echo "NOTE: PICO_SDK_PATH is not set. CMake will attempt to fetch it automatically via git."
        export PICO_SDK_FETCH_FROM_GIT=on
    fi
fi

mkdir -p build
cd build

echo "Configuring CMake project..."
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DPICO_BOARD=pico_w

echo "Building firmware..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ -f "stadia_ps5_bridge.uf2" ]; then
    echo ""
    echo "Build SUCCESSFUL!"
    echo "UF2 Firmware File: $(pwd)/stadia_ps5_bridge.uf2"
    echo ""
    echo "To Flash your Pico WH:"
    echo "1. Hold the BOOTSEL button on the Pico WH while plugging into USB."
    echo "2. Copy 'stadia_ps5_bridge.uf2' onto the mounted RPI-RP2 drive."
fi
