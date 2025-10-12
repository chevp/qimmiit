# Nanook Engine

**A lightweight, standalone central orchestrator library for C++ applications**

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Dependencies](https://img.shields.io/badge/dependencies-none-green)
![Build](https://img.shields.io/badge/build-passing-green)

## Overview

**Nanook** (named after the Inuit word for "polar bear" 🐻‍❄️) is a central orchestrator architecture library that provides a single agent to coordinate multiple specialized modules. Similar to how Chrome orchestrates tabs, VS Code manages extensions, or Unreal Engine coordinates subsystems.

## Key Features

✅ **Zero Dependencies** - Pure C++17 standard library, no external dependencies
✅ **Modular Architecture** - Clean IModule interface with automatic dependency resolution
✅ **Global Services** - ConfigManager, LogManager, EventBus, TaskScheduler, MemoryManager
✅ **Optional Configuration** - Works without config files, supports INI-style configs
✅ **Event-Driven** - Pub/sub messaging system for inter-module communication
✅ **Reusable Library** - Ship as `nanook.lib` (4.3 MB static library)

## Quick Start

### 1. Build the Library

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 2. Link in Your Project

**CMakeLists.txt:**
```cmake
add_subdirectory(nanook)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE nanook)
```

### 3. Use in Your Code

```cpp
#include <nanook/core/CentralAgent.hpp>
#include <nanook/modules/RendererModule.hpp>

int main() {
    auto& agent = cryo::engine::CentralAgent::getInstance();
    agent.registerModule(std::make_shared<cryo::engine::RendererModule>());

    cryo::engine::EngineConfig config;
    if (agent.initialize(config)) {
        agent.shutdown();
    }
}
```

## Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Quick reference guide
- **[USAGE_EXAMPLE.md](USAGE_EXAMPLE.md)** - Integration examples
- **[BUILD_SUMMARY.md](BUILD_SUMMARY.md)** - Build information
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Detailed design (15,000+ lines)

## Build Outputs

| File | Size | Description |
|------|------|-------------|
| **`nanook.lib`** | 4.3 MB | **Complete static library** - Use this |
| `nanook_core.lib` | 3.5 MB | Core only (for custom modules) |
| `nanook-cli.exe` | 114 KB | Example CLI application |

---

**Nanook Engine v1.0.0** - A lightweight central orchestrator for C++ 🐻‍❄️
