#!/bin/bash
# Build script for Avannaaq-001 (Linux/macOS)

set -e

echo "========================================"
echo "Avannaaq-001 Build Script"
echo "========================================"
echo

# Configuration
BUILD_TYPE="Release"
BUILD_SERVER="ON"
BUILD_CLIENT="ON"
BUILD_CMS="ON"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --no-server)
            BUILD_SERVER="OFF"
            shift
            ;;
        --no-client)
            BUILD_CLIENT="OFF"
            shift
            ;;
        --no-cms)
            BUILD_CMS="OFF"
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug        Build in Debug mode (default: Release)"
            echo "  --no-server    Skip server build"
            echo "  --no-client    Skip client build"
            echo "  --no-cms       Skip CMS tools build"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check for vcpkg
if [ -z "$VCPKG_ROOT" ]; then
    if [ -d "/opt/vcpkg" ]; then
        VCPKG_ROOT="/opt/vcpkg"
    elif [ -d "$HOME/vcpkg" ]; then
        VCPKG_ROOT="$HOME/vcpkg"
    else
        echo "ERROR: VCPKG_ROOT not set and vcpkg not found"
        echo "Please set VCPKG_ROOT environment variable"
        exit 1
    fi
fi

echo "Using vcpkg from: $VCPKG_ROOT"
echo
echo "Configuration:"
echo "  Build Type: $BUILD_TYPE"
echo "  Build Server: $BUILD_SERVER"
echo "  Build Client: $BUILD_CLIENT"
echo "  Build CMS: $BUILD_CMS"
echo

# Configure
echo "Configuring..."
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DAVANNAAQ_BUILD_SERVER="$BUILD_SERVER" \
    -DAVANNAAQ_BUILD_CLIENT="$BUILD_CLIENT" \
    -DAVANNAAQ_BUILD_CMS="$BUILD_CMS"

echo
echo "Building..."
cmake --build build --config "$BUILD_TYPE" -j$(nproc)

echo
echo "========================================"
echo "Build Complete!"
echo "========================================"
echo
echo "Binaries located in: build/bin/"
echo
echo "To run the server:"
echo "  cd build/bin"
echo "  ./avannaaq-server --config ../../config/server_dev.json"
echo
