# Qimmiit

**"Qimmiit"** (Inuktitut: "Hunde" / "Dogs", plural of *Qimmiq*) — A coordinated but individualized engine architecture, like a dog sled team working together towards a common goal.

## Overview

Qimmiit is a modular game engine framework built on Arctic graphics technology, featuring a Vulkan-based rendering pipeline and a flexible module system. The architecture emphasizes coordination between independent components, each with their own capabilities and responsibilities, working together as a unified system.

### Core Philosophy

Like sled dogs in a team, each module in Qimmiit:
- Operates independently with its own responsibilities
- Coordinates with other modules through a central agent
- Contributes to the overall system's forward motion
- Can be added, removed, or replaced without disrupting the team

## Architecture

### Nanook - The Module Orchestrator

**Nanook** serves as an example implementation of the Qimmiit architecture. Named after the mythological "master of bears" in Inuit mythology, Nanook orchestrates the coordination between engine modules.

#### Key Components

**CentralAgent** - The coordination hub
- Manages module lifecycle (initialization, update, shutdown)
- Routes messages between modules
- Provides access to core systems (config, logging, events, tasks, memory)

**Core Systems**
- **ConfigManager**: XML-based configuration management
- **LogManager**: Hierarchical logging system
- **EventBus**: Pub/sub messaging between modules
- **TaskScheduler**: Multi-threaded task execution
- **MemoryManager**: Custom memory allocation pools

**Module Types**
- **Renderer Module**: Graphics rendering coordination
- **Cache Module**: Scene and asset caching
- **Asset Module**: Asset loading and management
- **HTTP Module**: Web communication and remote services
- **Compute Module**: Background computation services (gRPC)

### Arctic Graphics Engine

The underlying graphics technology featuring:
- **Vulkan** rendering API
- **gRPC** services for distributed computation
- **Protocol Buffers** for data serialization
- **glTF** scene loading
- **ImGui** for debugging interfaces

#### Major Subsystems

- **arctic-formats**: Data format loaders (AuthoredScene, RuntimeEntity)
- **coregfx**: Core graphics primitives and Vulkan wrappers
- **cryo-cache**: Scene/asset caching library (DLL)
- **cryo-asset**: Bundled game assets library (DLL)
- **cryo-tooling**: Development tools (shader compiler, asset manager, CLI)
- **cryo-compute-engine**: Background computation service (gRPC)
- **cryo-studio-server**: Studio rendering server

## Building

### Prerequisites

- **CMake** 3.20 or higher
- **C++17** compatible compiler
- **vcpkg** package manager (recommended) or system package manager
- **Vulkan SDK**

### Dependencies

```
vulkan, vulkan-memory-allocator, glm, gli, tinygltf, imgui,
protobuf, grpc, tinyxml2, glfw3, glew, stb, sqlite3, curl,
gtest, glslang, shaderc, spirv-reflect
```

### Build Instructions

#### Windows (Visual Studio 2022)

```bash
# Using default preset (Debug x64)
cmake --preset default
cmake --build build --config Debug

# Or use release preset
cmake --preset release-x64
cmake --build build-release --config Release
```

#### Linux/macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Build Options

- `BUILD_NANOOK_CLI`: Build the nanook command-line interface (default: ON)
- `BUILD_NANOOK_SHARED`: Build nanook as a shared library (default: OFF)
- `BUILD_TESTS`: Build C++ unit and integration tests (default: OFF)
- `BUILD_DESKTOP_JNI`: Build desktop JNI binding for Quarkus (default: OFF)

### Platform Support

- Windows (Vulkan via Win32)
- Linux (Vulkan via XCB)
- macOS (Vulkan via MoltenVK/Metal)

## Usage

### Nanook CLI

The nanook CLI provides tools for engine introspection and management:

```bash
# Generate engine specification
nanook-cli spec

# Generate JSON specification
nanook-cli spec --format json --output spec.json

# List all registered modules
nanook-cli modules list

# Get detailed module information
nanook-cli modules info RendererModule

# Validate configuration
nanook-cli config validate --config arctic.config.xml

# Show version
nanook-cli --version
```

### Creating Custom Modules

```cpp
#include <nanook/core/IModule.hpp>

class MyModule : public cryo::engine::IModule {
public:
    bool initialize(cryo::engine::CentralAgent& agent) override {
        // Module initialization
        return true;
    }

    void update(float deltaTime) override {
        // Per-frame updates
    }

    void shutdown() override {
        // Cleanup
    }

    ModuleSpec getSpecification() const override {
        ModuleSpec spec;
        spec.name = "MyModule";
        spec.version = "1.0.0";
        spec.type = ModuleType::Logic;
        spec.description = "Custom game logic module";
        return spec;
    }
};
```

### Registering Modules

```cpp
#include <nanook/core/CentralAgent.hpp>
#include "MyModule.hpp"

int main() {
    auto& agent = cryo::engine::CentralAgent::getInstance();

    cryo::engine::EngineConfig config;
    config.configFile = "arctic.config.xml";
    config.logLevel = "INFO";
    config.threadCount = 8;
    config.memoryPoolSizeMB = 1024;

    agent.initialize(config);
    agent.registerModule(std::make_shared<MyModule>());

    while (agent.isRunning()) {
        agent.update(0.016f); // ~60 FPS
    }

    agent.shutdown();
    return 0;
}
```

## Project Structure

```
qimmiit/
├── nanook/                    # Nanook module orchestrator
│   ├── src/
│   │   ├── core/             # Core engine systems
│   │   ├── modules/          # Built-in module implementations
│   │   └── cli/              # Command-line interface
│   └── CMakeLists.txt
├── arctic-formats/            # Data format loaders
├── coregfx/                   # Core graphics library
├── cryo-cache/                # Asset caching system
├── cryo-asset/                # Asset management
├── cryo-tooling/              # Development tools
├── cryo-studio-server/        # Rendering server
├── services/
│   └── cryo-compute-engine/   # Computation service (gRPC)
├── apps/
│   ├── foundation/
│   │   ├── data-driven-renderer/
│   │   └── shader-graph-renderer/
│   └── desktop/
│       └── polar-draw/        # Studio UI client
├── minimal-vulkan-benchmark/  # Performance testing
├── tests/                     # Unit and integration tests
├── CMakeLists.txt            # Root build configuration
├── CMakePresets.json         # CMake presets
├── vcpkg.json                # Package dependencies
└── README.md                 # This file
```

## Configuration

The engine uses XML-based configuration via `arctic.config.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Config>
    <Engine>
        <LogLevel>INFO</LogLevel>
        <ThreadCount>8</ThreadCount>
        <MemoryPoolSizeMB>1024</MemoryPoolSizeMB>
    </Engine>
    <Renderer>
        <Backend>Vulkan</Backend>
        <Width>1920</Width>
        <Height>1080</Height>
        <VSync>true</VSync>
    </Renderer>
    <!-- Module-specific sections -->
</Config>
```

## Development

### Adding New Modules

1. Create module implementation inheriting from `IModule`
2. Add source files to `nanook/src/modules/`
3. Update `nanook/CMakeLists.txt` to build the module
4. Register module in your application code

### Running Tests

```bash
cmake --preset default -DBUILD_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build
```

### Code Generation

Protocol buffer and gRPC code is automatically generated during build:
- Proto files are co-located with their libraries
- Each library handles its own proto generation
- Generated files appear in `<BUILD_DIR>/generated/`

## License

[License information needed]

## Contributing

[Contributing guidelines needed]

## Credits

Inspired by the Inuit tradition of dog sledding, where coordination and individual capability come together to achieve remarkable journeys across challenging terrain.

---

*"Like a well-coordinated dog sled team, each module pulls its weight while the musher (CentralAgent) guides the journey."*