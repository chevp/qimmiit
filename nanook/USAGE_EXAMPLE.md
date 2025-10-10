# Using Nanook Library in Your Project

Nanook is now available as a reusable static library (`nanook.lib`) that can be integrated into any C++ project.

## Build Outputs

After building, you'll find these libraries in `build/Release/`:

- **`nanook.lib`** (4.3 MB) - Complete static library (core + all modules)
- **`nanook_core.lib`** (3.5 MB) - Core engine only (no modules)
- **`nanook_*_module.lib`** - Individual module libraries
- **`nanook-cli.exe`** (114 KB) - Example CLI application

## Integration Methods

### Method 1: Link against complete `nanook.lib` (Recommended)

This is the simplest approach - link against the complete library that includes everything.

**CMakeLists.txt:**
```cmake
# Add nanook library
add_subdirectory(nanook)

# Your executable
add_executable(my_app main.cpp)

# Link against complete nanook library
target_link_libraries(my_app PRIVATE nanook)
```

**main.cpp:**
```cpp
#include <nanook/core/CentralAgent.hpp>
#include <nanook/core/EngineConfig.hpp>
#include <nanook/modules/RendererModule.hpp>

using namespace cryo::engine;

int main() {
    auto& agent = CentralAgent::getInstance();

    // Configure engine
    EngineConfig config;
    config.configFile = "myapp.config.ini";
    config.threadCount = 8;

    // Register modules
    agent.registerModule(std::make_shared<RendererModule>());

    // Initialize and run
    if (agent.initialize(config)) {
        // Your application logic here
        agent.shutdown();
    }

    return 0;
}
```

### Method 2: Link against `nanook_core.lib` + custom modules

Use this if you want to create your own custom modules without the default ones.

**CMakeLists.txt:**
```cmake
add_subdirectory(nanook)

add_executable(my_app main.cpp MyCustomModule.cpp)

target_link_libraries(my_app PRIVATE
    nanook_core  # Core only
)
```

**MyCustomModule.hpp:**
```cpp
#include <nanook/core/IModule.hpp>

class MyCustomModule : public cryo::engine::IModule {
public:
    bool initialize(CentralAgent& agent) override;
    void update(float deltaTime) override;
    void shutdown() override;

    std::string getName() const override { return "MyCustomModule"; }
    ModuleType getType() const override { return ModuleType::CUSTOM; }
    std::vector<std::string> getDependencies() const override { return {}; }

    void handleMessage(const Message& msg) override;
    ModuleSpec getSpecification() const override;
};
```

### Method 3: Pre-built library (without CMake integration)

If you've already built nanook and want to use it in a separate project:

**CMakeLists.txt:**
```cmake
# Specify nanook library location
set(NANOOK_DIR "path/to/nanook")

# Include directories
include_directories(${NANOOK_DIR}/src)

# Link against pre-built library
add_executable(my_app main.cpp)

target_link_libraries(my_app PRIVATE
    ${NANOOK_DIR}/build/Release/nanook.lib
)
```

### Method 4: Install and use system-wide

```bash
# Install nanook to system directories
cd nanook/build
cmake --install . --prefix /usr/local

# Or on Windows (Admin required)
cmake --install . --prefix "C:/Program Files/nanook"
```

**CMakeLists.txt:**
```cmake
# Find installed nanook
find_package(nanook REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE nanook::nanook)
```

## Example: Arctic Game Client Integration

Here's how to integrate nanook into the arctic-game-client project:

**arctic-game-client/CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(arctic-game-client)

# Add nanook as subdirectory
add_subdirectory(nanook)

# Your existing coregfx library
add_library(coregfx STATIC ...)

# Link coregfx against nanook
target_link_libraries(coregfx PUBLIC nanook)

# Your renderer executable
add_executable(arctic-renderer main.cpp)
target_link_libraries(arctic-renderer PRIVATE coregfx nanook)
```

**Usage in arctic-game-client:**
```cpp
// main.cpp
#include <nanook/core/CentralAgent.hpp>
#include <coregfx/renderer.hpp>

int main() {
    using namespace cryo::engine;

    auto& agent = CentralAgent::getInstance();

    // Register arctic-specific modules
    agent.registerModule(std::make_shared<ArcticRendererModule>());
    agent.registerModule(std::make_shared<ArcticCacheModule>());

    EngineConfig config;
    config.configFile = "arctic.config.ini";

    if (agent.initialize(config)) {
        // Run arctic game loop
        while (running) {
            agent.update(deltaTime);
        }
        agent.shutdown();
    }

    return 0;
}
```

## Module Development

Creating your own nanook module:

**MyGameModule.hpp:**
```cpp
#include <nanook/core/IModule.hpp>
#include <nanook/core/CentralAgent.hpp>

namespace mygame {

class GameLogicModule : public cryo::engine::IModule {
private:
    cryo::engine::CentralAgent* agent_;

public:
    bool initialize(cryo::engine::CentralAgent& agent) override {
        agent_ = &agent;

        // Access global services
        auto& config = agent.getConfigManager();
        auto& log = agent.getLogManager();
        auto& events = agent.getEventBus();

        log.info("GameLogicModule", "Initializing game logic...");

        // Subscribe to events
        events.subscribe(cryo::engine::EventType::CUSTOM,
            [this](const auto& e) { onGameEvent(e); });

        return true;
    }

    void update(float deltaTime) override {
        // Game logic update
    }

    void shutdown() override {
        agent_->getLogManager().info("GameLogicModule", "Shutting down");
    }

    std::string getName() const override { return "GameLogicModule"; }

    cryo::engine::ModuleType getType() const override {
        return cryo::engine::ModuleType::CUSTOM;
    }

    std::vector<std::string> getDependencies() const override {
        return {"RendererModule", "AssetModule"};
    }

    void handleMessage(const cryo::engine::Message& msg) override {
        // Handle inter-module messages
    }

    cryo::engine::ModuleSpec getSpecification() const override {
        return {
            "GameLogicModule",
            "1.0.0",
            cryo::engine::ModuleType::CUSTOM,
            "Handles game logic and state management",
            {"RendererModule", "AssetModule"}
        };
    }

private:
    void onGameEvent(const cryo::engine::Event& e) {
        // Handle game events
    }
};

} // namespace mygame
```

**Usage:**
```cpp
agent.registerModule(std::make_shared<mygame::GameLogicModule>());
```

## Configuration

Nanook uses INI-style config files:

**myapp.config.ini:**
```ini
[Engine]
name = MyApplication
version = 1.0.0

[Core]
thread_count = 8
memory_pool_mb = 512

[Logging]
log_level = DEBUG
log_to_console = true

# Custom module configuration
[MyGameModule]
difficulty = hard
max_players = 4
enable_networking = true
```

**Access in your module:**
```cpp
bool MyGameModule::initialize(CentralAgent& agent) {
    auto& config = agent.getConfigManager();

    std::string difficulty = config.getString("MyGameModule.difficulty", "normal");
    int maxPlayers = config.getInt("MyGameModule.max_players", 1);
    bool networking = config.getBool("MyGameModule.enable_networking", false);

    // Use configuration values...
    return true;
}
```

## Library Dependencies

**Nanook has ZERO external dependencies**:
- Pure C++17 standard library
- No Vulkan, no OpenGL, no tinyxml2, no Boost
- Completely standalone

**This means:**
- Easy to integrate into any project
- Fast compilation
- No dependency hell
- Cross-platform compatible

## Next Steps

1. **Build nanook**: `cmake -B build && cmake --build build --config Release`
2. **Link against `nanook.lib`** in your project
3. **Include headers**: `#include <nanook/core/CentralAgent.hpp>`
4. **Register modules** and initialize the engine
5. **Extend with custom modules** specific to your application

---

**Nanook** - A reusable central orchestrator library for C++ applications 🐻‍❄️
