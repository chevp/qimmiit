# Nanook Engine - Quick Start Guide

**Nanook** is a central orchestrator architecture engine designed to coordinate multiple specialized modules.

## Build Status

✅ **Successfully built with CMake**
- No external dependencies (standalone)
- Pure C++17 implementation
- Cross-platform compatible

## Building

```bash
# Configure CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

## Running

```bash
# Run with default configuration
./build/Release/nanook-cli.exe

# Run with config file
./build/Release/nanook-cli.exe --config nanook.config.ini

# Show help
./build/Release/nanook-cli.exe --help

# Show version
./build/Release/nanook-cli.exe --version
```

## Architecture Overview

**Central Agent Pattern** - Single orchestrator coordinates all modules:

```
┌─────────────────────────────────────────┐
│         CentralAgent (Singleton)        │
│  - ConfigManager                        │
│  - LogManager                           │
│  - EventBus (pub/sub)                   │
│  - TaskScheduler                        │
│  - MemoryManager                        │
└──────────────┬──────────────────────────┘
               │
               ├──> RendererModule
               ├──> CacheModule
               ├──> AssetModule
               ├──> HttpModule
               └──> ComputeModule
```

## Key Features

### 1. No External Dependencies
- ConfigManager uses INI-style parser (no tinyxml2)
- Standard C++17 only
- Standalone static libraries

### 2. Optional Configuration
- Works without config file (in-memory defaults)
- Supports INI-style config files
- Command-line configuration override

### 3. Module System
- IModule interface for all modules
- Automatic dependency resolution (topological sort)
- Lifecycle management (initialize → update → shutdown)

### 4. Global Services
- **ConfigManager**: INI file parsing
- **LogManager**: Timestamped logging with levels
- **EventBus**: Pub/sub event system
- **TaskScheduler**: Multi-threaded task scheduling
- **MemoryManager**: Memory pool management

## Configuration File Format

Nanook uses simple INI-style configuration:

```ini
# nanook.config.ini

[Engine]
name = Nanook
version = 1.0.0
thread_count = 8

[Logging]
log_level = DEBUG
log_to_console = true

[Renderer]
api = Vulkan
validation_layers = true
```

Keys are accessed with dot notation: `Engine.thread_count`

## Module Registration

```cpp
auto& agent = CentralAgent::getInstance();

// Register modules
agent.registerModule(std::make_shared<CacheModule>());
agent.registerModule(std::make_shared<AssetModule>());
agent.registerModule(std::make_shared<RendererModule>());

// Initialize (respects dependencies)
EngineConfig config;
config.threadCount = 4;
agent.initialize(config);
```

## Inter-Module Communication

### Event Bus (Pub/Sub)
```cpp
// Subscribe
auto& eventBus = agent.getEventBus();
uint64_t id = eventBus.subscribe(EventType::ASSET_LOADED, [](const Event& e) {
    std::cout << "Asset loaded: " << e.source << std::endl;
});

// Publish
eventBus.publish(EventType::ASSET_LOADED, "AssetModule", payload);
```

### Module Access
```cpp
// Get module by type
auto renderer = agent.getModule<RendererModule>();

// Get module by name
auto cache = agent.getModuleByName("CacheModule");
```

## Project Structure

```
nanook/
├── src/
│   ├── core/                  # Core engine components
│   │   ├── CentralAgent.*     # Main orchestrator
│   │   ├── ConfigManager.*    # INI config parser
│   │   ├── LogManager.*       # Logging system
│   │   ├── EventBus.*         # Event pub/sub
│   │   ├── TaskScheduler.*    # Task scheduling
│   │   └── MemoryManager.*    # Memory management
│   │
│   ├── modules/               # Module implementations
│   │   ├── RendererModule.*
│   │   ├── CacheModule.*
│   │   ├── AssetModule.*
│   │   ├── HttpModule.*
│   │   └── ComputeModule.*
│   │
│   └── cli/                   # Command-line interface
│       └── main_simple.cpp
│
├── CMakeLists.txt             # CMake build configuration
├── nanook.config.ini          # Example config file
├── README.md                  # Full documentation
├── ARCHITECTURE.md            # Detailed design docs
└── QUICKSTART.md              # This file
```

## Example Output

```
===========================================
  Cryo Engine CLI - Version 1.0.0
  Central Orchestrator Architecture
===========================================

Initializing Nanook Engine...
Using config file: nanook.config.ini

[2025-10-10 15:00:54] [INFO ] [CentralAgent] Initializing Cryo Engine...
[2025-10-10 15:00:54] [INFO ] [CacheModule] Initialized successfully
[2025-10-10 15:00:54] [INFO ] [RendererModule] Initialized successfully
[2025-10-10 15:00:54] [INFO ] [CentralAgent] Cryo Engine initialized successfully

Engine initialized successfully!
Registered modules: 5

Modules:
  - HttpModule [HTTP]
  - CacheModule [CACHE]
  - AssetModule [ASSET]
  - RendererModule [RENDERER]
  - ComputeModule [COMPUTE]
```

## Next Steps

1. **Add Real Implementations**: Current modules are stubs - implement actual functionality
2. **Integrate with Arctic**: Use nanook as orchestrator for arctic-game-client
3. **Add Vulkan Renderer**: Implement RendererModule with Vulkan backend
4. **Add HTTP Server**: Implement HttpModule with REST API
5. **Add Asset Pipeline**: Implement CacheModule and AssetModule with GLTF support

## Design Philosophy

**Nanook follows the Central Agent pattern:**
- ✅ Single point of control (CentralAgent)
- ✅ Global services accessible to all modules
- ✅ Automatic dependency resolution
- ✅ Event-driven inter-module communication
- ✅ No external dependencies for core engine
- ✅ Optional file-based configuration

**Similar to:**
- Chrome's multi-process architecture
- Unreal Engine's module system
- VS Code's extension host
- Docker daemon

---

**Nanook** - An Inuit name meaning "polar bear", representing strength and coordination in harsh environments 🐻‍❄️
