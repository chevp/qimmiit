# Qimmiit SDK Development

Development SDK for the Qimmiit Engine ecosystem.

## Overview

The Qimmiit SDK provides a unified interface to all Qimmiit modules:
- **coregfx** - Vulkan rendering engine
- **cryo-asset** - Asset management system
- **cryo-cache** - Caching infrastructure
- **akutik** - Storage backend
- **cryo-tooling** - Development tools

## Building

### Prerequisites
- CMake 3.20+
- C++20 compatible compiler
- Vulkan SDK
- vcpkg (for dependencies)

### Build Instructions

**Important:** You need to specify the vcpkg toolchain file.

**Windows:**
```bash
# Set vcpkg path (adjust as needed)
set VCPKG_ROOT=C:/path/to/vcpkg

# Configure
cmake -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Build with examples
cmake -B build -DQIMMIIT_SDK_BUILD_EXAMPLES=ON -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

**Linux/macOS:**
```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Usage

### In Your CMakeLists.txt

```cmake
# Add SDK to your project
add_subdirectory(path/to/qimmiit-sdk-dev)

# Link against SDK
target_link_libraries(your-target PRIVATE Qimmiit::SDK)
```

### Basic Example

```cpp
#include <qimmiit/Renderer.hpp>
#include <qimmiit/AssetManager.hpp>

int main() {
    // Initialize renderer
    auto renderer = qimmiit::Renderer::create();

    // Load assets
    auto assetManager = qimmiit::AssetManager::create();
    auto scene = assetManager->loadScene("scene.gltf");

    // Render loop
    while (renderer->isRunning()) {
        renderer->beginFrame();
        renderer->render(scene);
        renderer->endFrame();
    }

    return 0;
}
```

## Examples

Example projects are available in the `examples/` directory:

1. **basic-renderer** - Minimal rendering setup
2. **asset-loader** - Asset loading and management
3. **studio-integration** - Full studio environment

Build examples:
```bash
cmake -B build -DQIMMIIT_SDK_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/basic-renderer-example
```

## Architecture

```
qimmiit-sdk-dev/
├── CMakeLists.txt       # Main SDK configuration
├── examples/            # Example applications
│   ├── basic-renderer/
│   ├── asset-loader/
│   └── studio-integration/
└── include/             # SDK unified headers (future)
```

The SDK aggregates modules from `../../modules/`:
- `modules/rendering/coregfx`
- `modules/assets/cryo-asset`
- `modules/assets/cryo-cache`
- `modules/toolings/akutik`
- `modules/toolings/cryo-tooling`

## Generating Release SDK

To create a distributable SDK package:

```bash
cd ../packaging
cmake -P generate-release-sdk.cmake
```

This will create a packaged SDK in `dist/sdk/qimmiit-sdk-{version}-{platform}/`

## For Qimmiit Studio Prime

The `qimmiit-studio-prime` application uses this SDK:

```cmake
# In apps/qimmiit-studio-prime/CMakeLists.txt
add_subdirectory(../../sdk/qimmiit-sdk-dev qimmiit-sdk)
target_link_libraries(qimmiit-studio-prime PRIVATE Qimmiit::SDK)
```

## Module Availability

The SDK automatically detects which modules are available. Check the CMake configuration output:

```
Qimmiit SDK Dev Configuration Summary:
  Version: 1.0.0
  Modules:
    - coregfx:      TRUE
    - cryo-asset:   TRUE
    - cryo-cache:   TRUE
    - akutik:       TRUE
    - cryo-tooling: TRUE
```

## Contributing

When adding new modules to the SDK:

1. Add the module to `modules/`
2. Update `sdk/qimmiit-sdk-dev/CMakeLists.txt` to include it
3. Update examples if needed
4. Regenerate release SDK

## License

[Your License Here]
