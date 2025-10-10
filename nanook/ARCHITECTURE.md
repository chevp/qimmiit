# Cryo Engine Architecture

**Detailed Technical Design Document**

---

## Table of Contents

1. [Overview](#overview)
2. [Core Architecture](#core-architecture)
3. [Central Agent Design](#central-agent-design)
4. [Module System](#module-system)
5. [Inter-Module Communication](#inter-module-communication)
6. [Configuration Management](#configuration-management)
7. [Task Scheduling](#task-scheduling)
8. [Memory Management](#memory-management)
9. [Error Handling](#error-handling)
10. [Performance Considerations](#performance-considerations)
11. [Security Model](#security-model)
12. [Testing Strategy](#testing-strategy)

---

## Overview

### Architectural Pattern

Cryo Engine implements a **Central Orchestrator** pattern where a single agent coordinates all subsystems. This contrasts with traditional library-based architectures where each component is independent.

```
Traditional Library Architecture:
App → LibA → LibB → LibC  (Dependency chain)

Central Orchestrator Architecture:
       ┌─ LibA
       ├─ LibB
Agent ─┼─ LibC  (Agent coordinates all)
       ├─ LibD
       └─ LibE
```

### Design Goals

1. **Unified Runtime**: Single executable with all functionality
2. **Centralized Control**: Agent manages all module lifecycles
3. **Simplified Integration**: Modules plug into the agent, not each other
4. **Global Services**: Shared logging, configuration, profiling
5. **Workflow Orchestration**: Complex multi-module tasks coordinated by agent

---

## Core Architecture

### System Layers

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                    │
│  (main.cpp, command-line interface, UI bootstrap)       │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                  Central Agent Layer                    │
│                                                         │
│  ┌─────────────────────────────────────────────────┐    │
│  │ CentralAgent (Singleton)                        │    │
│  │  - Module registry & lifecycle                  │    │
│  │  - Configuration management                     │    │
│  │  - Event bus & messaging                        │    │
│  │  - Task scheduler                               │    │
│  │  - Global services (logging, profiling, etc.)   │    │
│  └─────────────────────────────────────────────────┘    │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                   Module Layer                          │
│                                                         │
│  ┌───────────┐  ┌───────────┐  ┌───────────┐            │
│  │ Renderer  │  │   Cache   │  │   Asset   │            │
│  │  Module   │  │  Module   │  │  Module   │  ...       │
│  └───────────┘  └───────────┘  └───────────┘            │
│                                                         │
│  Each module implements IModule interface               │
│  Modules register with CentralAgent at startup          │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                  Foundation Layer                       │
│  (Vulkan, GLTF, Protobuf, SQLite, cpp-httplib, etc.)    │
└─────────────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Responsibilities | Dependencies |
|-----------|-----------------|--------------|
| **CentralAgent** | Module lifecycle, global state, coordination | None (singleton) |
| **ConfigManager** | Load/parse configuration, provide access | tinyxml2 |
| **LogManager** | Centralized logging, log levels, outputs | None |
| **EventBus** | Pub/sub messaging between modules | None |
| **TaskScheduler** | Background task execution, priorities | Thread pool |
| **MemoryManager** | Shared memory pools, tracking | None |
| **RendererModule** | Vulkan rendering, GLTF loading, PBR | Vulkan, GLM |
| **CacheModule** | Scene/asset caching, SQLite storage | SQLite, Protobuf |
| **AssetModule** | Asset processing, validation | TinyGLTF |
| **HttpModule** | REST API server, WebSocket | cpp-httplib |
| **ComputeModule** | Background tasks (shader compile, baking) | Thread pool |

---

## Central Agent Design

### CentralAgent Class

**File**: `cryo-engine/src/core/CentralAgent.hpp`

```cpp
#pragma once

#include "IModule.hpp"
#include "ConfigManager.hpp"
#include "LogManager.hpp"
#include "EventBus.hpp"
#include "TaskScheduler.hpp"
#include "MemoryManager.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace cryo {
namespace engine {

class CentralAgent {
public:
    // Singleton access
    static CentralAgent& getInstance();

    // Delete copy/move constructors (singleton pattern)
    CentralAgent(const CentralAgent&) = delete;
    CentralAgent& operator=(const CentralAgent&) = delete;

    // Lifecycle management
    bool initialize(const EngineConfig& config);
    void shutdown();
    void update(float deltaTime);

    // Module management
    void registerModule(std::shared_ptr<IModule> module);
    void unregisterModule(const std::string& moduleName);

    template<typename T>
    std::shared_ptr<T> getModule(const std::string& name) {
        auto it = modules_.find(name);
        if (it != modules_.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    // Global services
    ConfigManager& getConfigManager() { return configManager_; }
    LogManager& getLogManager() { return logManager_; }
    EventBus& getEventBus() { return eventBus_; }
    TaskScheduler& getTaskScheduler() { return taskScheduler_; }
    MemoryManager& getMemoryManager() { return memoryManager_; }

    // Inter-module communication
    void sendMessage(const std::string& targetModule, const Message& msg);
    void broadcastMessage(const Message& msg);

    // Engine state
    bool isRunning() const { return running_; }
    void requestShutdown() { running_ = false; }

    // Statistics
    size_t getModuleCount() const { return modules_.size(); }
    std::vector<std::string> getModuleNames() const;

private:
    CentralAgent() = default;
    ~CentralAgent() = default;

    // Module initialization (topological sort by dependencies)
    bool initializeModules();
    std::vector<std::shared_ptr<IModule>> topologicalSortModules();

    // State
    bool running_ = false;
    std::unordered_map<std::string, std::shared_ptr<IModule>> modules_;
    std::vector<std::shared_ptr<IModule>> moduleInitOrder_;

    // Global services
    ConfigManager configManager_;
    LogManager logManager_;
    EventBus eventBus_;
    TaskScheduler taskScheduler_;
    MemoryManager memoryManager_;
};

} // namespace engine
} // namespace cryo
```

### Initialization Sequence

```cpp
// cryo-engine/src/core/CentralAgent.cpp

bool CentralAgent::initialize(const EngineConfig& config) {
    // 1. Initialize global services
    if (!logManager_.initialize(config.logLevel, config.logOutputs)) {
        std::cerr << "Failed to initialize LogManager" << std::endl;
        return false;
    }

    logManager_.info("CentralAgent", "Initializing Cryo Engine...");

    if (!configManager_.initialize(config.configFile)) {
        logManager_.error("CentralAgent", "Failed to load configuration");
        return false;
    }

    if (!eventBus_.initialize()) {
        logManager_.error("CentralAgent", "Failed to initialize EventBus");
        return false;
    }

    if (!taskScheduler_.initialize(config.threadCount)) {
        logManager_.error("CentralAgent", "Failed to initialize TaskScheduler");
        return false;
    }

    if (!memoryManager_.initialize(config.memoryPoolSizeMB)) {
        logManager_.error("CentralAgent", "Failed to initialize MemoryManager");
        return false;
    }

    // 2. Initialize modules in dependency order
    if (!initializeModules()) {
        logManager_.error("CentralAgent", "Failed to initialize modules");
        return false;
    }

    running_ = true;
    logManager_.info("CentralAgent", "Cryo Engine initialized successfully");
    return true;
}

bool CentralAgent::initializeModules() {
    // Topological sort modules by dependencies
    moduleInitOrder_ = topologicalSortModules();

    // Initialize each module
    for (auto& module : moduleInitOrder_) {
        logManager_.info("CentralAgent",
            "Initializing module: " + module->getName());

        if (!module->initialize(*this)) {
            logManager_.error("CentralAgent",
                "Failed to initialize module: " + module->getName());
            return false;
        }
    }

    return true;
}

std::vector<std::shared_ptr<IModule>> CentralAgent::topologicalSortModules() {
    // Kahn's algorithm for topological sort
    std::unordered_map<std::string, int> inDegree;
    std::unordered_map<std::string, std::vector<std::string>> adjList;

    // Build dependency graph
    for (const auto& [name, module] : modules_) {
        inDegree[name] = 0;
        adjList[name] = {};
    }

    for (const auto& [name, module] : modules_) {
        for (const auto& dep : module->getDependencies()) {
            adjList[dep].push_back(name);
            inDegree[name]++;
        }
    }

    // Find modules with no dependencies
    std::queue<std::string> queue;
    for (const auto& [name, degree] : inDegree) {
        if (degree == 0) {
            queue.push(name);
        }
    }

    // Process queue (topological sort)
    std::vector<std::shared_ptr<IModule>> sorted;
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        sorted.push_back(modules_[current]);

        for (const auto& neighbor : adjList[current]) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }

    // Check for circular dependencies
    if (sorted.size() != modules_.size()) {
        logManager_.error("CentralAgent", "Circular dependency detected!");
        // Return partial sort (will fail during initialization)
    }

    return sorted;
}
```

---

## Module System

### IModule Interface

**File**: `cryo-engine/src/core/IModule.hpp`

```cpp
#pragma once

#include <string>
#include <vector>

namespace cryo {
namespace engine {

class CentralAgent;  // Forward declaration

enum class ModuleType {
    RENDERER,
    CACHE,
    ASSET,
    HTTP,
    COMPUTE,
    CUSTOM
};

class IModule {
public:
    virtual ~IModule() = default;

    /**
     * Initialize the module
     * @param agent Reference to the central agent
     * @return true if successful, false otherwise
     */
    virtual bool initialize(CentralAgent& agent) = 0;

    /**
     * Update the module (called every frame)
     * @param deltaTime Time since last frame (seconds)
     */
    virtual void update(float deltaTime) = 0;

    /**
     * Shutdown the module (called in reverse initialization order)
     */
    virtual void shutdown() = 0;

    /**
     * Get the module name (must be unique)
     */
    virtual std::string getName() const = 0;

    /**
     * Get the module type
     */
    virtual ModuleType getType() const = 0;

    /**
     * Get list of module dependencies (by name)
     * Used for topological sorting during initialization
     */
    virtual std::vector<std::string> getDependencies() const = 0;

    /**
     * Handle incoming message from another module
     * @param msg Message to process
     */
    virtual void handleMessage(const Message& msg) = 0;
};

} // namespace engine
} // namespace cryo
```

### Example Module Implementation

**File**: `cryo-engine/src/modules/RendererModule.hpp`

```cpp
#pragma once

#include "../core/IModule.hpp"
#include <coregfx/core/ocean_base.hpp>  // VulkanContext, etc.
#include <memory>

namespace cryo {
namespace engine {

class RendererModule : public IModule {
public:
    RendererModule() = default;
    ~RendererModule() override = default;

    // IModule interface
    bool initialize(CentralAgent& agent) override;
    void update(float deltaTime) override;
    void shutdown() override;

    std::string getName() const override { return "RendererModule"; }
    ModuleType getType() const override { return ModuleType::RENDERER; }

    std::vector<std::string> getDependencies() const override {
        return {"CacheModule", "AssetModule"};  // Depends on cache & asset
    }

    void handleMessage(const Message& msg) override;

    // Renderer-specific API
    void loadScene(const std::string& sceneFile);
    void renderFrame();
    void setCameraPosition(const glm::vec3& pos);
    void setCameraRotation(const glm::vec3& rot);
    void takeScreenshot(const std::string& filename);

private:
    CentralAgent* agent_ = nullptr;
    std::unique_ptr<ocean::VulkanContext> vulkanContext_;
    bool vulkanInitialized_ = false;

    // Internal methods
    bool initVulkan();
    void cleanupVulkan();
};

} // namespace engine
} // namespace cryo
```

**File**: `cryo-engine/src/modules/RendererModule.cpp`

```cpp
#include "RendererModule.hpp"
#include "../core/CentralAgent.hpp"
#include <coregfx/core/ocean_log.hpp>

namespace cryo {
namespace engine {

bool RendererModule::initialize(CentralAgent& agent) {
    agent_ = &agent;

    auto& log = agent.getLogManager();
    auto& config = agent.getConfigManager();

    log.info("RendererModule", "Initializing Vulkan renderer...");

    // Get configuration
    bool validation = config.getBool("renderer.vulkan-validation", false);
    bool vsync = config.getBool("renderer.vsync", true);
    int msaa = config.getInt("renderer.msaa-samples", 1);

    log.info("RendererModule", "Vulkan validation: " + std::to_string(validation));
    log.info("RendererModule", "VSync: " + std::to_string(vsync));
    log.info("RendererModule", "MSAA samples: " + std::to_string(msaa));

    // Initialize Vulkan
    if (!initVulkan()) {
        log.error("RendererModule", "Failed to initialize Vulkan");
        return false;
    }

    // Subscribe to events
    agent.getEventBus().subscribe(EventType::ASSET_READY,
        [this](const Event& e) {
            agent_->getLogManager().info("RendererModule",
                "Asset ready: " + e.data.toString());
            // Load asset into GPU
        });

    vulkanInitialized_ = true;
    log.info("RendererModule", "Initialized successfully");
    return true;
}

void RendererModule::update(float deltaTime) {
    if (!vulkanInitialized_) return;

    // Render frame
    renderFrame();
}

void RendererModule::shutdown() {
    auto& log = agent_->getLogManager();
    log.info("RendererModule", "Shutting down...");

    if (vulkanInitialized_) {
        cleanupVulkan();
        vulkanInitialized_ = false;
    }

    log.info("RendererModule", "Shutdown complete");
}

void RendererModule::handleMessage(const Message& msg) {
    auto& log = agent_->getLogManager();

    switch (msg.type) {
        case MessageType::LOAD_SCENE:
            log.info("RendererModule", "Loading scene: " + msg.payload.toString());
            loadScene(msg.payload.toString());
            break;

        case MessageType::SET_CAMERA:
            log.info("RendererModule", "Setting camera position");
            // Extract camera data from payload
            break;

        case MessageType::TAKE_SCREENSHOT:
            log.info("RendererModule", "Taking screenshot");
            takeScreenshot(msg.payload.toString());
            break;

        default:
            log.warn("RendererModule", "Unknown message type: " +
                     std::to_string(static_cast<int>(msg.type)));
            break;
    }
}

bool RendererModule::initVulkan() {
    // Initialize Vulkan context (similar to existing coregfx code)
    vulkanContext_ = std::make_unique<ocean::VulkanContext>();

    if (!vulkanContext_->initialize()) {
        return false;
    }

    return true;
}

void RendererModule::cleanupVulkan() {
    if (vulkanContext_) {
        vulkanContext_->cleanup();
        vulkanContext_.reset();
    }
}

void RendererModule::renderFrame() {
    // Rendering logic (existing coregfx rendering code)
    if (vulkanContext_) {
        vulkanContext_->renderFrame();
    }
}

} // namespace engine
} // namespace cryo
```

---

## Inter-Module Communication

### Event Bus

**File**: `cryo-engine/src/core/EventBus.hpp`

```cpp
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

namespace cryo {
namespace engine {

enum class EventType {
    SCENE_LOADED,
    ASSET_READY,
    ASSET_FAILED,
    CACHE_HIT,
    CACHE_MISS,
    HTTP_REQUEST,
    HTTP_RESPONSE,
    CUSTOM
};

struct Event {
    EventType type;
    std::string source;  // Module that published the event
    Payload data;        // Event-specific data
    uint64_t timestamp;  // Milliseconds since epoch
};

using EventCallback = std::function<void(const Event&)>;

class EventBus {
public:
    bool initialize();
    void shutdown();

    /**
     * Subscribe to an event type
     * @param type Event type to subscribe to
     * @param callback Function to call when event is published
     * @return Subscription ID (used for unsubscribing)
     */
    uint64_t subscribe(EventType type, EventCallback callback);

    /**
     * Unsubscribe from an event
     * @param subscriptionId ID returned from subscribe()
     */
    void unsubscribe(uint64_t subscriptionId);

    /**
     * Publish an event (thread-safe)
     * @param event Event to publish
     */
    void publish(const Event& event);

    /**
     * Publish an event with automatic timestamp
     */
    void publish(EventType type, const std::string& source, const Payload& data);

private:
    std::mutex mutex_;
    std::unordered_map<EventType, std::vector<std::pair<uint64_t, EventCallback>>> subscribers_;
    uint64_t nextSubscriptionId_ = 1;
};

} // namespace engine
} // namespace cryo
```

### Direct Messaging

**File**: `cryo-engine/src/core/Message.hpp`

```cpp
#pragma once

#include <string>
#include "Payload.hpp"

namespace cryo {
namespace engine {

enum class MessageType {
    LOAD_SCENE,
    SET_CAMERA,
    TAKE_SCREENSHOT,
    CACHE_REQUEST,
    CACHE_RESPONSE,
    ASSET_PROCESS,
    HTTP_REQUEST,
    CUSTOM
};

struct Message {
    MessageType type;
    std::string source;      // Module that sent the message
    std::string target;      // Module to receive the message (empty = broadcast)
    Payload payload;         // Message-specific data
    uint64_t timestamp;      // Milliseconds since epoch
    uint64_t requestId = 0;  // For request/response correlation
};

} // namespace engine
} // namespace cryo
```

---

## Configuration Management

### ConfigManager

**File**: `cryo-engine/src/core/ConfigManager.hpp`

```cpp
#pragma once

#include <string>
#include <unordered_map>
#include <tinyxml2.h>

namespace cryo {
namespace engine {

class ConfigManager {
public:
    bool initialize(const std::string& configFile);
    void shutdown();

    // Type-safe getters
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;

    // Check if key exists
    bool hasKey(const std::string& key) const;

    // Reload configuration at runtime
    bool reload();

private:
    std::string configFile_;
    std::unordered_map<std::string, std::string> values_;

    void parseXML(tinyxml2::XMLDocument& doc);
    void parseElement(tinyxml2::XMLElement* elem, const std::string& prefix);
};

} // namespace engine
} // namespace cryo
```

### Configuration File Format

**File**: `arctic.config.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<cryo-engine version="1.0">
    <!-- Engine global settings -->
    <engine>
        <thread-count>8</thread-count>
        <log-level>DEBUG</log-level>
        <memory-pool-mb>512</memory-pool-mb>
    </engine>

    <!-- Renderer module -->
    <renderer>
        <vulkan-validation>true</vulkan-validation>
        <vsync>true</vsync>
        <msaa-samples>4</msaa-samples>
        <default-width>1920</default-width>
        <default-height>1080</default-height>
    </renderer>

    <!-- Cache module -->
    <cache>
        <enabled>true</enabled>
        <max-size-gb>10</max-size-gb>
        <ttl-days>7</ttl-days>
        <strategy>LRU</strategy>
        <cache-dir>cache/</cache-dir>
    </cache>

    <!-- Asset module -->
    <asset>
        <asset-root>assets/vkpbr5</asset-root>
        <enable-compression>true</enable-compression>
        <texture-format>DDS</texture-format>
    </asset>

    <!-- HTTP module -->
    <http-server>
        <enabled>true</enabled>
        <host>0.0.0.0</host>
        <port>52009</port>
        <cors>true</cors>
        <max-connections>100</max-connections>
    </http-server>

    <!-- Compute module -->
    <compute>
        <worker-threads>4</worker-threads>
        <queue-size>1000</queue-size>
    </compute>
</cryo-engine>
```

---

## Task Scheduling

### TaskScheduler

**File**: `cryo-engine/src/core/TaskScheduler.hpp`

```cpp
#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace cryo {
namespace engine {

enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

using Task = std::function<void()>;

struct ScheduledTask {
    Task task;
    TaskPriority priority;
    std::chrono::steady_clock::time_point scheduledTime;
    uint64_t taskId;

    bool operator<(const ScheduledTask& other) const {
        // Higher priority first, then earlier scheduled time
        if (priority != other.priority) {
            return priority < other.priority;
        }
        return scheduledTime > other.scheduledTime;
    }
};

class TaskScheduler {
public:
    bool initialize(size_t threadCount);
    void shutdown();

    /**
     * Schedule immediate task
     */
    uint64_t schedule(Task task, TaskPriority priority = TaskPriority::NORMAL);

    /**
     * Schedule delayed task
     */
    uint64_t scheduleDelayed(Task task, std::chrono::milliseconds delay,
                            TaskPriority priority = TaskPriority::NORMAL);

    /**
     * Schedule recurring task
     */
    uint64_t scheduleRecurring(Task task, std::chrono::milliseconds interval,
                              TaskPriority priority = TaskPriority::NORMAL);

    /**
     * Cancel scheduled task
     */
    bool cancelTask(uint64_t taskId);

    /**
     * Get number of pending tasks
     */
    size_t getPendingTaskCount() const;

private:
    void workerThread();

    std::vector<std::thread> workers_;
    std::priority_queue<ScheduledTask> taskQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    bool running_ = false;
    uint64_t nextTaskId_ = 1;
};

} // namespace engine
} // namespace cryo
```

---

## Memory Management

### MemoryManager

**File**: `cryo-engine/src/core/MemoryManager.hpp`

```cpp
#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <mutex>

namespace cryo {
namespace engine {

class MemoryManager {
public:
    bool initialize(size_t poolSizeMB);
    void shutdown();

    /**
     * Allocate memory from shared pool
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

    /**
     * Free allocated memory
     */
    void free(void* ptr);

    /**
     * Get total allocated memory (bytes)
     */
    size_t getTotalAllocated() const;

    /**
     * Get peak allocated memory (bytes)
     */
    size_t getPeakAllocated() const;

    /**
     * Get number of active allocations
     */
    size_t getAllocationCount() const;

private:
    struct AllocationHeader {
        size_t size;
        void* ptr;
    };

    std::unique_ptr<uint8_t[]> pool_;
    size_t poolSize_ = 0;
    size_t allocated_ = 0;
    size_t peakAllocated_ = 0;
    mutable std::mutex mutex_;
    std::vector<AllocationHeader> allocations_;
};

} // namespace engine
} // namespace cryo
```

---

## Error Handling

### Error Codes

**File**: `cryo-engine/src/core/ErrorCodes.hpp`

```cpp
#pragma once

namespace cryo {
namespace engine {

enum class ErrorCode {
    SUCCESS = 0,

    // Agent errors
    AGENT_INIT_FAILED = 1000,
    AGENT_SHUTDOWN_FAILED = 1001,

    // Module errors
    MODULE_INIT_FAILED = 2000,
    MODULE_DEPENDENCY_FAILED = 2001,
    MODULE_CIRCULAR_DEPENDENCY = 2002,
    MODULE_NOT_FOUND = 2003,

    // Configuration errors
    CONFIG_FILE_NOT_FOUND = 3000,
    CONFIG_PARSE_ERROR = 3001,
    CONFIG_VALIDATION_FAILED = 3002,

    // Resource errors
    RESOURCE_NOT_FOUND = 4000,
    RESOURCE_LOAD_FAILED = 4001,
    OUT_OF_MEMORY = 4002,

    // Vulkan errors
    VULKAN_INIT_FAILED = 5000,
    VULKAN_DEVICE_LOST = 5001,

    // Network errors
    HTTP_SERVER_FAILED = 6000,
    HTTP_REQUEST_FAILED = 6001,

    // Custom
    UNKNOWN_ERROR = 9999
};

const char* errorCodeToString(ErrorCode code);

} // namespace engine
} // namespace cryo
```

### Exception Handling

```cpp
// Modules report errors via return values + logging
// Central agent catches exceptions and logs them

try {
    if (!module->initialize(*this)) {
        logManager_.error("CentralAgent",
            "Module initialization failed: " + module->getName());
        return false;
    }
} catch (const std::exception& e) {
    logManager_.error("CentralAgent",
        "Exception during module init: " + std::string(e.what()));
    return false;
}
```

---

## Performance Considerations

### Optimization Strategies

1. **Module Update Order**: Critical modules (renderer) update first
2. **Task Batching**: Batch similar tasks to reduce context switching
3. **Memory Pooling**: Reduce allocation overhead with shared pools
4. **Event Filtering**: Modules subscribe only to relevant events
5. **Lock-Free Queues**: Use lock-free data structures for message passing
6. **Zero-Copy Messaging**: Pass references instead of copying data

### Profiling Integration

```cpp
// Built-in profiler accessible via agent
agent.getProfiler().beginSection("RenderFrame");
renderFrame();
agent.getProfiler().endSection("RenderFrame");

// Dump profiling results
agent.getProfiler().dumpResults("profile.json");
```

---

## Security Model

### Sandboxing

- Modules cannot directly access other modules' internal state
- All communication goes through CentralAgent
- Agent validates all inter-module messages

### Configuration Validation

```cpp
// ConfigManager validates values
int threadCount = config.getInt("engine.thread-count", 4);
if (threadCount < 1 || threadCount > 64) {
    logManager_.warn("ConfigManager",
        "Invalid thread count, clamping to [1, 64]");
    threadCount = std::clamp(threadCount, 1, 64);
}
```

---

## Testing Strategy

### Unit Testing Modules

```cpp
// Test module in isolation with mock agent
class MockAgent : public CentralAgent {
    // Override methods for testing
};

TEST(RendererModuleTest, Initialize) {
    MockAgent mockAgent;
    RendererModule renderer;

    EXPECT_TRUE(renderer.initialize(mockAgent));
    EXPECT_TRUE(renderer.getName() == "RendererModule");
}
```

### Integration Testing

```cpp
// Test full agent + modules
TEST(CentralAgentTest, ModuleLifecycle) {
    CentralAgent& agent = CentralAgent::getInstance();
    EngineConfig config = loadTestConfig();

    agent.registerModule(std::make_shared<RendererModule>());
    EXPECT_TRUE(agent.initialize(config));

    agent.update(0.016f);  // Simulate frame

    agent.shutdown();
}
```

---

## Summary

Cryo Engine provides a **unified runtime** with:

✅ **Centralized control** - Agent manages all modules
✅ **Simplified integration** - Modules plug into agent, not each other
✅ **Global services** - Shared logging, config, scheduling
✅ **Workflow orchestration** - Complex tasks coordinated by agent
✅ **Single deployment** - One executable with all functionality

**Trade-offs**:
- ❌ Monolithic architecture (harder to reuse modules)
- ❌ Tight coupling to agent (modules depend on CentralAgent interface)
- ❌ Longer build times (rebuild all on changes)

**Best suited for**: Unified applications (game engines, studio tools) where coordinated workflows and simple deployment are priorities.

---

*Cryo Engine Architecture - Version 1.0*