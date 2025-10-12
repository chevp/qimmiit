# Sakku Native Binary

High-performance C++ binary for game asset processing.

## What It Does

The native binary (`sakku-cli.exe`) handles computationally intensive tasks:

- GLTF model optimization
- Texture compression (BC7, ASTC, etc.)
- Shader compilation (GLSL → SPIR-V)
- Mesh optimization
- Asset validation

## Building

### Prerequisites

- CMake 3.20+
- vcpkg
- C++17 compiler (MSVC, GCC, or Clang)

### Build Instructions

```bash
# Install dependencies via vcpkg
vcpkg install nlohmann-json libcurl

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Binary will be in: build/bin/sakku-cli
```

### Build for All Platforms

```bash
# Windows (x64)
cmake -B build-win64 -A x64
cmake --build build-win64 --config Release

# Linux (x64)
cmake -B build-linux64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux64

# macOS (ARM64)
cmake -B build-macos-arm64 -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build-macos-arm64 --config Release
```

## Command Line Interface

The binary has its own CLI when run directly:

```bash
# Build assets
sakku-cli build --optimize --compress

# Process GLTF
sakku-cli process-gltf input.gltf output.gltf --optimize

# Compress texture
sakku-cli compress-texture input.png output.bc7 --format bc7

# Version
sakku-cli --version
```

## Integration with Node.js Wrapper

The Node.js wrapper (`../nodejs/`) spawns this binary and communicates via:

1. **Command-line arguments** - Pass options
2. **Standard output** - Receive results
3. **Exit codes** - Success/failure status

Example from Node.js:

```javascript
const { execa } = require('execa');

// Node.js spawns native binary
const result = await execa('./binaries/win32-x64/sakku-cli.exe', [
  'build',
  '--optimize',
  '--compress'
]);

console.log(result.stdout); // Binary output
```

## Structure

```
native/
├── CMakeLists.txt       # Build configuration
├── include/             # Header files
│   └── sakku/
│       ├── FileUtils.hpp
│       ├── ManifestManager.hpp
│       └── ...
└── src/                 # Source files
    ├── main.cpp        # CLI entry point
    ├── commands/       # Command implementations
    └── ...
```

## See Also

- [../nodejs/](../nodejs/) - Node.js wrapper that calls this binary
- [../README.md](../README.md) - Main Sakku documentation
