# Central Orchestrator Architecture for CoreGFX

**How the Agent Pattern Can Transform CoreGFX into a Modular Graphics Engine**

---

## Table of Contents

1. [Overview](#overview)
2. [Current CoreGFX Architecture](#current-coregfx-architecture)
3. [The Central Orchestrator Pattern](#the-central-orchestrator-pattern)
4. [Applying the Pattern to CoreGFX](#applying-the-pattern-to-coregfx)
5. [Proposed Architecture](#proposed-architecture)
6. [Implementation Guide](#implementation-guide)
7. [Benefits](#benefits)
8. [Migration Strategy](#migration-strategy)
9. [Code Examples](#code-examples)

---

## Overview

CoreGFX currently operates as a monolithic Vulkan rendering library. By applying the **Central Orchestrator Architecture** (also known as the **Agent Pattern**), we can transform it into a modular, extensible graphics engine where independent rendering components coordinate through a central agent.

### Key Concept

Just as **Nanook** orchestrates engine modules (Renderer, Cache, Asset), a **CoreGFX Agent** could orchestrate graphics subsystems (Vulkan Backend, PBR Renderer, Shadow System, Post-Processing, GLTF Loader, etc.).

---

## Current CoreGFX Architecture

### Current Structure

```
coregfx/
├── src/
│   ├── core/
│   │   ├── ocean_base.cpp      # Monolithic base class
│   │   ├── ocean_swap_chain.cpp
│   │   └── vulkan_context.cpp
│   ├── pbr/                    # PBR rendering (tightly coupled)
│   ├── gltf/                   # GLTF loading (tightly coupled)
│   ├── overlay/                # UI overlays (tightly coupled)
│   └── imstudio/               # ImGui integration (tightly coupled)
```

### Problems with Current Design

1. **Tight Coupling** - All systems are directly interconnected
2. **Difficult to Test** - Can't test individual systems in isolation
3. **Hard to Extend** - Adding new rendering features requires modifying core classes
4. **Initialization Order** - Complex dependencies between systems
5. **State Management** - Shared state scattered across multiple classes
6. **No Plugin System** - Can't add/remove rendering features at runtime

---

## The Central Orchestrator Pattern

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│              Central Agent (Graphics Orchestrator)          │
│                                                             │
│  - Manages graphics module lifecycle                       │
│  - Routes messages between modules                         │
│  - Provides access to core systems                         │
│  - Coordinates rendering pipeline                          │
└─────────────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
        ▼                 ▼                 ▼
   ┌─────────┐      ┌─────────┐      ┌─────────┐
   │ Vulkan  │      │   PBR   │      │  GLTF   │
   │ Backend │      │ Renderer│      │ Loader  │
   │ Module  │      │ Module  │      │ Module  │
   └─────────┘      └─────────┘      └─────────┘
```

### Key Principles

1. **Independent Modules** - Each graphics subsystem is a self-contained module
2. **Message Passing** - Modules communicate via messages, not direct calls
3. **Dependency Management** - Agent resolves module dependencies automatically
4. **Lifecycle Management** - Agent controls initialization, update, and shutdown order
5. **Loose Coupling** - Modules only depend on the agent interface, not each other

---

## Applying the Pattern to CoreGFX

### Graphics Modules (Examples)

Each major CoreGFX subsystem becomes an independent module:

| Module | Responsibility | Dependencies |
|--------|----------------|--------------|
| **VulkanBackendModule** | Vulkan instance, device, queues | None |
| **SwapChainModule** | Swapchain, framebuffers | VulkanBackendModule |
| **PBRRendererModule** | PBR material rendering | VulkanBackendModule, SwapChainModule |
| **ShadowMapModule** | Shadow mapping system | VulkanBackendModule |
| **PostProcessModule** | Post-processing effects | SwapChainModule |
| **GLTFLoaderModule** | GLTF scene loading | VulkanBackendModule |
| **ImGuiModule** | ImGui overlay rendering | SwapChainModule |
| **TextureManagerModule** | Texture loading/caching | VulkanBackendModule |

---

## Proposed Architecture

### High-Level Structure

```
coregfx/
├── include/
│   └── coregfx/
│       ├── agent/
│       │   ├── GraphicsAgent.hpp        # Central orchestrator
│       │   ├── IGraphicsModule.hpp      # Module interface
│       │   └── GraphicsMessage.hpp      # Message types
│       └── modules/
│           ├── VulkanBackendModule.hpp
│           ├── PBRRendererModule.hpp
│           ├── ShadowMapModule.hpp
│           └── GLTFLoaderModule.hpp
├── src/
│   ├── agent/
│   │   └── GraphicsAgent.cpp
│   └── modules/
│       ├── VulkanBackendModule.cpp
│       ├── PBRRendererModule.cpp
│       ├── ShadowMapModule.cpp
│       └── GLTFLoaderModule.cpp
```

### Module Lifecycle

```
1. Registration Phase
   ↓
2. Dependency Resolution (topological sort)
   ↓
3. Initialize Modules (in dependency order)
   ↓
4. Update Loop (every frame)
   │
   ├─→ Update Phase (prepare for rendering)
   ├─→ Render Phase (issue draw calls)
   └─→ Present Phase (swap buffers)
   ↓
5. Shutdown (reverse order)
```

---

## Implementation Guide

### Step 1: Define IGraphicsModule Interface

```cpp
namespace coregfx {
namespace agent {

enum class ModuleType {
    BACKEND,      // Vulkan/DirectX backend
    RENDERER,     // PBR, forward, deferred
    LOADER,       // GLTF, OBJ, textures
    POST_PROCESS, // Bloom, AA, tone mapping
    UI,           // ImGui, overlays
    SHADOW,       // Shadow mapping
    LIGHTING      // Light management
};

class IGraphicsModule {
public:
    virtual ~IGraphicsModule() = default;

    // Lifecycle
    virtual bool initialize(GraphicsAgent& agent) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(VkCommandBuffer cmd) = 0;
    virtual void shutdown() = 0;

    // Module info
    virtual std::string getName() const = 0;
    virtual ModuleType getType() const = 0;
    virtual std::vector<std::string> getDependencies() const = 0;

    // Message handling
    virtual void handleMessage(const GraphicsMessage& msg) = 0;
};

} // namespace agent
} // namespace coregfx
```

### Step 2: Create GraphicsAgent

```cpp
class GraphicsAgent {
public:
    static GraphicsAgent& getInstance();

    // Module management
    void registerModule(std::shared_ptr<IGraphicsModule> module);
    void unregisterModule(const std::string& name);

    template<typename T>
    std::shared_ptr<T> getModule(const std::string& name);

    // Lifecycle
    bool initialize(const GraphicsConfig& config);
    void update(float deltaTime);
    void render(VkCommandBuffer cmd);
    void shutdown();

    // Resource access
    VkDevice getDevice() const { return device_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    VkQueue getGraphicsQueue() const { return graphicsQueue_; }

    // Message passing
    void sendMessage(const std::string& targetModule, const GraphicsMessage& msg);
    void broadcastMessage(const GraphicsMessage& msg);

private:
    VkInstance instance_;
    VkDevice device_;
    VkPhysicalDevice physicalDevice_;
    VkQueue graphicsQueue_;

    std::unordered_map<std::string, std::shared_ptr<IGraphicsModule>> modules_;
    std::vector<std::shared_ptr<IGraphicsModule>> renderOrder_;
};
```

### Step 3: Implement VulkanBackendModule

```cpp
class VulkanBackendModule : public IGraphicsModule {
public:
    bool initialize(GraphicsAgent& agent) override {
        // Create Vulkan instance
        createInstance();

        // Pick physical device
        pickPhysicalDevice();

        // Create logical device
        createLogicalDevice();

        return true;
    }

    void update(float deltaTime) override {
        // Backend doesn't need per-frame updates
    }

    void render(VkCommandBuffer cmd) override {
        // Backend doesn't issue draw calls
    }

    void shutdown() override {
        vkDestroyDevice(device_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }

    std::string getName() const override { return "VulkanBackend"; }
    ModuleType getType() const override { return ModuleType::BACKEND; }

    std::vector<std::string> getDependencies() const override {
        return {}; // No dependencies
    }

    void handleMessage(const GraphicsMessage& msg) override {
        // Handle device-related messages
    }

private:
    VkInstance instance_;
    VkDevice device_;
    VkPhysicalDevice physicalDevice_;
};
```

### Step 4: Implement PBRRendererModule

```cpp
class PBRRendererModule : public IGraphicsModule {
public:
    bool initialize(GraphicsAgent& agent) override {
        agent_ = &agent;

        // Get Vulkan backend
        auto backend = agent.getModule<VulkanBackendModule>("VulkanBackend");
        device_ = backend->getDevice();

        // Get swapchain
        auto swapchain = agent.getModule<SwapChainModule>("SwapChain");

        // Create PBR pipeline
        createPipeline();

        return true;
    }

    void update(float deltaTime) override {
        // Update PBR uniforms, lighting, etc.
        updateUniformBuffers();
    }

    void render(VkCommandBuffer cmd) override {
        // Bind PBR pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

        // Bind descriptors
        vkCmdBindDescriptorSets(cmd, ...);

        // Draw scene with PBR materials
        for (auto& mesh : scene_->meshes) {
            vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
        }
    }

    void shutdown() override {
        vkDestroyPipeline(device_, pipeline_, nullptr);
    }

    std::string getName() const override { return "PBRRenderer"; }
    ModuleType getType() const override { return ModuleType::RENDERER; }

    std::vector<std::string> getDependencies() const override {
        return {"VulkanBackend", "SwapChain"};
    }

    void handleMessage(const GraphicsMessage& msg) override {
        if (msg.type == MessageType::SCENE_LOADED) {
            scene_ = msg.data.scene;
        }
    }

private:
    GraphicsAgent* agent_;
    VkDevice device_;
    VkPipeline pipeline_;
    Scene* scene_;
};
```

---

## Benefits

### 1. **Modularity**
- Add/remove rendering features without touching core code
- Swap implementations (e.g., forward renderer → deferred renderer)

### 2. **Testability**
- Test each module in isolation
- Mock dependencies easily
- Unit test individual rendering systems

### 3. **Extensibility**
- Plugin system: load rendering modules from DLLs
- Users can add custom rendering techniques
- Easy to experiment with new features

### 4. **Maintainability**
- Clear separation of concerns
- Each module has well-defined responsibility
- Easier to debug (isolate issues to specific modules)

### 5. **Performance**
- Only load modules you need
- Reduce memory footprint
- Parallel module updates

### 6. **Reusability**
- Modules can be shared between projects
- Package individual modules as libraries
- Mix and match rendering techniques

---

## Migration Strategy

### Phase 1: Create Agent Infrastructure
1. Implement `IGraphicsModule` interface
2. Implement `GraphicsAgent` class
3. Add module registration system
4. Add dependency resolution

### Phase 2: Extract Core Systems
1. Convert `ocean_base` → `VulkanBackendModule`
2. Convert swapchain code → `SwapChainModule`
3. Convert command buffer management → `CommandBufferModule`

### Phase 3: Extract Rendering Features
1. Extract PBR rendering → `PBRRendererModule`
2. Extract shadow mapping → `ShadowMapModule`
3. Extract post-processing → `PostProcessModule`

### Phase 4: Extract Loaders
1. Extract GLTF loading → `GLTFLoaderModule`
2. Extract texture loading → `TextureManagerModule`
3. Extract material system → `MaterialManagerModule`

### Phase 5: Compatibility Layer
1. Create `Ocean` wrapper class that uses agent internally
2. Maintain backward compatibility with existing code
3. Gradually deprecate old API

---

## Code Examples

### Example 1: Creating a Graphics Application

```cpp
#include <coregfx/agent/GraphicsAgent.hpp>
#include <coregfx/modules/VulkanBackendModule.hpp>
#include <coregfx/modules/SwapChainModule.hpp>
#include <coregfx/modules/PBRRendererModule.hpp>
#include <coregfx/modules/GLTFLoaderModule.hpp>

int main() {
    // Get the graphics agent
    auto& agent = coregfx::agent::GraphicsAgent::getInstance();

    // Configure the engine
    coregfx::GraphicsConfig config;
    config.window = window;
    config.enableValidation = true;

    // Initialize agent
    agent.initialize(config);

    // Register modules (order doesn't matter - agent resolves dependencies)
    agent.registerModule(std::make_shared<VulkanBackendModule>());
    agent.registerModule(std::make_shared<SwapChainModule>());
    agent.registerModule(std::make_shared<PBRRendererModule>());
    agent.registerModule(std::make_shared<GLTFLoaderModule>());
    agent.registerModule(std::make_shared<ShadowMapModule>());
    agent.registerModule(std::make_shared<ImGuiModule>());

    // Load scene
    auto loader = agent.getModule<GLTFLoaderModule>("GLTFLoader");
    loader->loadScene("models/scene.gltf");

    // Main loop
    while (!windowShouldClose(window)) {
        // Update all modules
        agent.update(deltaTime);

        // Begin rendering
        VkCommandBuffer cmd = beginFrame();

        // Render all modules
        agent.render(cmd);

        // End rendering
        endFrame(cmd);
    }

    // Cleanup
    agent.shutdown();

    return 0;
}
```

### Example 2: Custom Rendering Module

```cpp
#include <coregfx/agent/IGraphicsModule.hpp>

class CustomToonShaderModule : public IGraphicsModule {
public:
    bool initialize(GraphicsAgent& agent) override {
        // Get required dependencies
        auto backend = agent.getModule<VulkanBackendModule>("VulkanBackend");
        device_ = backend->getDevice();

        // Create toon shader pipeline
        createToonPipeline();

        return true;
    }

    void render(VkCommandBuffer cmd) override {
        // Bind toon shader pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, toonPipeline_);

        // Draw with toon shading
        drawToonShaded(cmd);
    }

    std::vector<std::string> getDependencies() const override {
        return {"VulkanBackend", "SwapChain"};
    }

    // ... implement other interface methods
};

// Usage:
agent.registerModule(std::make_shared<CustomToonShaderModule>());
```

### Example 3: Message Passing Between Modules

```cpp
// ShadowMapModule sends message when shadows are ready
class ShadowMapModule : public IGraphicsModule {
    void render(VkCommandBuffer cmd) override {
        // Render shadow maps
        renderShadows(cmd);

        // Notify other modules that shadows are ready
        GraphicsMessage msg;
        msg.type = MessageType::SHADOWS_READY;
        msg.data.shadowMap = shadowMapTexture_;

        agent_->broadcastMessage(msg);
    }
};

// PBRRendererModule receives shadow map
class PBRRendererModule : public IGraphicsModule {
    void handleMessage(const GraphicsMessage& msg) override {
        if (msg.type == MessageType::SHADOWS_READY) {
            shadowMap_ = msg.data.shadowMap;
        }
    }

    void render(VkCommandBuffer cmd) override {
        // Use shadow map in PBR rendering
        bindShadowMap(shadowMap_);
        renderWithShadows(cmd);
    }
};
```

### Example 4: Dynamic Module Loading

```cpp
// Load modules at runtime based on configuration
void loadGraphicsModules(GraphicsAgent& agent, const Config& config) {
    // Always load backend
    agent.registerModule(std::make_shared<VulkanBackendModule>());
    agent.registerModule(std::make_shared<SwapChainModule>());

    // Load renderer based on config
    if (config.renderer == "pbr") {
        agent.registerModule(std::make_shared<PBRRendererModule>());
    } else if (config.renderer == "forward") {
        agent.registerModule(std::make_shared<ForwardRendererModule>());
    } else if (config.renderer == "deferred") {
        agent.registerModule(std::make_shared<DeferredRendererModule>());
    }

    // Optional features
    if (config.enableShadows) {
        agent.registerModule(std::make_shared<ShadowMapModule>());
    }

    if (config.enablePostProcessing) {
        agent.registerModule(std::make_shared<PostProcessModule>());
    }

    if (config.enableUI) {
        agent.registerModule(std::make_shared<ImGuiModule>());
    }
}
```

---

## Comparison: Before and After

### Before (Monolithic)

```cpp
// Tightly coupled - hard to modify
class Ocean {
    VulkanContext vulkan;
    SwapChain swapchain;
    PBRRenderer pbr;
    ShadowMapper shadows;
    GLTFLoader loader;
    ImGuiOverlay imgui;

    void initialize() {
        // Complex initialization order
        vulkan.init();
        swapchain.init(vulkan);
        pbr.init(vulkan, swapchain);
        shadows.init(vulkan);
        loader.init(vulkan);
        imgui.init(vulkan, swapchain);
    }

    void render() {
        // Everything is tightly coupled
        shadows.renderShadows();
        pbr.render(shadows.getShadowMap());
        imgui.render();
    }
};
```

### After (Agent Pattern)

```cpp
// Modular - easy to extend
auto& agent = GraphicsAgent::getInstance();
agent.initialize(config);

// Just register modules - agent handles dependencies
agent.registerModule(std::make_shared<VulkanBackendModule>());
agent.registerModule(std::make_shared<SwapChainModule>());
agent.registerModule(std::make_shared<PBRRendererModule>());
agent.registerModule(std::make_shared<ShadowMapModule>());
agent.registerModule(std::make_shared<GLTFLoaderModule>());
agent.registerModule(std::make_shared<ImGuiModule>());

// Agent automatically:
// - Resolves dependencies
// - Initializes in correct order
// - Coordinates rendering
while (running) {
    agent.update(deltaTime);
    agent.render(cmd);
}

agent.shutdown();
```

---

## Conclusion

The **Central Orchestrator Architecture** transforms CoreGFX from a monolithic library into a modular, extensible graphics engine. By applying the same agent pattern used in Nanook, CoreGFX gains:

- ✅ **Modularity** - Independent, self-contained graphics subsystems
- ✅ **Extensibility** - Easy to add new rendering features
- ✅ **Testability** - Each module can be tested in isolation
- ✅ **Maintainability** - Clear separation of concerns
- ✅ **Flexibility** - Mix and match rendering techniques
- ✅ **Performance** - Load only what you need

This architecture enables CoreGFX to evolve from a rendering library into a full-featured graphics engine that can compete with modern engines while maintaining the flexibility to adapt to new rendering techniques and hardware.

---

**Next Steps:**
1. Prototype `GraphicsAgent` and `IGraphicsModule`
2. Extract `VulkanBackendModule` from ocean_base
3. Create proof-of-concept with 2-3 modules
4. Measure performance impact
5. Gradually migrate remaining systems

---

*"Like a well-coordinated graphics pipeline, each module processes its stage while the agent ensures smooth flow from input to final pixels."*
