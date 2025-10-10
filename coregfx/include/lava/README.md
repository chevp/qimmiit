# Lava - Reusable Vulkan Toolkit

## Overview

**Lava** is a collection of reusable, low-level Vulkan utilities and abstractions designed to simplify common Vulkan operations across the Arctic Game Client ecosystem. It provides thin wrappers and helper functions that reduce boilerplate while maintaining direct access to Vulkan's capabilities.

## Purpose

Lava serves as **Vulkan tooling** - not an orchestrator or specific implementation, but a library of reusable building blocks:

- **Low-level Vulkan helpers** - Wrapper utilities that reduce boilerplate
- **Common patterns** - Frequently used Vulkan operation sequences
- **Debug utilities** - Validation, logging, and diagnostic tools
- **Resource management** - RAII wrappers for Vulkan objects
- **State tracking** - Lightweight state management for Vulkan resources

## Architecture Philosophy

### What Lava IS:
- ✅ **Thin abstraction layer** over Vulkan API
- ✅ **Utility functions** for common Vulkan tasks
- ✅ **RAII wrappers** for Vulkan handles (VkDevice, VkBuffer, VkImage, etc.)
- ✅ **Helper classes** for initialization, synchronization, memory management
- ✅ **Debug and validation** tools for Vulkan development
- ✅ **Reusable across projects** - Used by coregfx, data-driven-renderer, elyrion, etc.

### What Lava IS NOT:
- ❌ **Not a rendering engine** - No scene graph, entity system, or rendering pipeline
- ❌ **Not an orchestrator** - No high-level scene management or asset loading
- ❌ **Not opinionated** - Doesn't enforce specific rendering approaches
- ❌ **Not a replacement for Vulkan** - Direct Vulkan API is still accessible

## Proposed Components

### 1. Core Initialization (`lava/core/`)
```cpp
// Device selection and initialization
class DeviceBuilder {
    DeviceBuilder& requireExtensions(std::vector<const char*> extensions);
    DeviceBuilder& requireFeatures(VkPhysicalDeviceFeatures features);
    VkDevice create();
};

// Swapchain creation helper
class SwapchainBuilder {
    SwapchainBuilder& setFormat(VkFormat format);
    SwapchainBuilder& setPresentMode(VkPresentModeKHR mode);
    VkSwapchainKHR create();
};

// Command pool management
class CommandPoolManager {
    VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel level);
    void resetPool();
};
```

### 2. Resource Management (`lava/resources/`)
```cpp
// RAII wrapper for VkBuffer
class Buffer {
    Buffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);
    ~Buffer(); // Automatic cleanup

    void upload(const void* data, VkDeviceSize size);
    void* map();
    void unmap();
};

// RAII wrapper for VkImage
class Image {
    Image(VkDevice device, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
    ~Image();

    void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    VkImageView createView(VkImageAspectFlags aspectMask);
};

// Memory allocator helper
class MemoryAllocator {
    VkDeviceMemory allocate(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties);
    void free(VkDeviceMemory memory);
};
```

### 3. Command Buffer Helpers (`lava/commands/`)
```cpp
// Single-time command execution
class ImmediateCommandBuffer {
    ImmediateCommandBuffer(VkDevice device, VkCommandPool pool, VkQueue queue);

    void execute(std::function<void(VkCommandBuffer)> commands);
};

// Command buffer recording helpers
namespace CommandHelpers {
    void beginRenderPass(VkCommandBuffer cmd, VkRenderPass renderPass, VkFramebuffer framebuffer);
    void copyBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void blitImage(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D extent);
}
```

### 4. Synchronization (`lava/sync/`)
```cpp
// Fence wrapper
class Fence {
    Fence(VkDevice device, bool signaled = false);
    ~Fence();

    void wait(uint64_t timeout = UINT64_MAX);
    void reset();
};

// Semaphore wrapper
class Semaphore {
    Semaphore(VkDevice device);
    ~Semaphore();

    VkSemaphore handle() const;
};

// Frame synchronization helper
class FrameSync {
    FrameSync(VkDevice device, uint32_t framesInFlight);

    void waitForFrame(uint32_t frameIndex);
    VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex);
    VkSemaphore getRenderFinishedSemaphore(uint32_t frameIndex);
    VkFence getInFlightFence(uint32_t frameIndex);
};
```

### 5. Pipeline Utilities (`lava/pipeline/`)
```cpp
// Shader module loader
class ShaderModule {
    ShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);
    ~ShaderModule();

    VkPipelineShaderStageCreateInfo getStageInfo(VkShaderStageFlagBits stage, const char* entryPoint = "main");
};

// Graphics pipeline builder
class GraphicsPipelineBuilder {
    GraphicsPipelineBuilder& addShaderStage(VkShaderStageFlagBits stage, VkShaderModule module);
    GraphicsPipelineBuilder& setVertexInput(const VkPipelineVertexInputStateCreateInfo& vertexInput);
    GraphicsPipelineBuilder& setViewport(VkExtent2D extent);
    GraphicsPipelineBuilder& setRasterization(VkPolygonMode polygonMode, VkCullModeFlags cullMode);

    VkPipeline build(VkRenderPass renderPass, VkPipelineLayout layout);
};

// Descriptor set helpers
class DescriptorSetBuilder {
    DescriptorSetBuilder& addBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages);
    VkDescriptorSetLayout buildLayout();
    VkDescriptorSet allocate(VkDescriptorPool pool);
};
```

### 6. Debug and Validation (`lava/debug/`)
```cpp
// Vulkan validation layer integration
class ValidationLayers {
    static std::vector<const char*> getRequiredLayers();
    static bool checkSupport();
    static VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance);
};

// Object naming for debugging
namespace DebugMarkers {
    void setObjectName(VkDevice device, uint64_t object, VkObjectType type, const char* name);
    void beginRegion(VkCommandBuffer cmd, const char* name, glm::vec4 color = {1, 1, 1, 1});
    void endRegion(VkCommandBuffer cmd);
}

// State dumping utility
class StateDumper {
    void dumpDeviceInfo(VkPhysicalDevice physicalDevice);
    void dumpMemoryInfo(VkPhysicalDevice physicalDevice);
    void dumpPipelineState(VkPipeline pipeline);
};
```

### 7. Extensions and Features (`lava/extensions/`)
```cpp
// Extension management
class ExtensionManager {
    bool isExtensionSupported(const char* extensionName);
    std::vector<const char*> getRequiredExtensions();
    void enableExtension(const char* extensionName);
};

// Feature detection
class FeatureDetector {
    bool supportsGeometryShader();
    bool supportsTessellationShader();
    bool supportsRayTracing();
    VkPhysicalDeviceFeatures getAvailableFeatures();
};
```

### 8. Memory Management (`lava/memory/`)
```cpp
// Staging buffer for uploads
class StagingBuffer {
    StagingBuffer(VkDevice device, VkDeviceSize size);

    void upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    void copyTo(VkBuffer dstBuffer, VkDeviceSize size);
};

// Memory usage tracking
class MemoryTracker {
    void trackAllocation(VkDeviceMemory memory, VkDeviceSize size, const char* name);
    void trackDeallocation(VkDeviceMemory memory);
    void printUsageReport();
};
```

## Design Principles

### 1. **Zero-Cost Abstractions**
- RAII wrappers have no runtime overhead
- Inline functions where appropriate
- No virtual function overhead unless necessary

### 2. **Explicit is Better Than Implicit**
- No hidden state or magic behavior
- Clear ownership semantics
- Predictable lifecycle management

### 3. **Composability**
- Small, focused classes that do one thing well
- Easy to combine utilities for complex operations
- No forced dependencies between components

### 4. **Fail Fast**
- Validate parameters at API boundaries
- Use assertions for developer errors
- Provide clear error messages

### 5. **Interoperability**
- Always expose underlying Vulkan handles
- Compatible with raw Vulkan API calls
- No lock-in - can mix Lava and raw Vulkan freely

## Integration with Existing Codebase

### Current Vulkan Code Locations:
- `coregfx/src/core/ocean_base.cpp` - Core Vulkan initialization
- `coregfx/src/pbr/PBR_VulkanApp.cpp` - PBR rendering pipeline
- `apps/foundation/data-driven-renderer/` - Data-driven shader renderer
- `elyrion.coregfx.renderer/` - Entity-based scene renderer

### Migration Strategy:
1. **Extract common patterns** from existing Vulkan code
2. **Create Lava utilities** for repeated operations
3. **Gradually refactor** existing code to use Lava
4. **Maintain backward compatibility** during transition
5. **Validate with tests** to ensure no regressions

### Example Refactoring:
**Before** (raw Vulkan):
```cpp
VkBufferCreateInfo bufferInfo = {};
bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
bufferInfo.size = sizeof(vertices);
bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

VkBuffer buffer;
vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

VkMemoryRequirements memRequirements;
vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

VkMemoryAllocateInfo allocInfo = {};
allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
allocInfo.allocationSize = memRequirements.size;
allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

VkDeviceMemory bufferMemory;
vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
vkBindBufferMemory(device, buffer, bufferMemory, 0);
```

**After** (with Lava):
```cpp
lava::Buffer vertexBuffer(device, sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
vertexBuffer.upload(vertices.data(), sizeof(vertices));
```

## Build Integration

### CMake Integration:
```cmake
# lava/CMakeLists.txt
add_library(lava STATIC
    src/core/DeviceBuilder.cpp
    src/resources/Buffer.cpp
    src/resources/Image.cpp
    src/commands/ImmediateCommandBuffer.cpp
    src/sync/Fence.cpp
    src/sync/Semaphore.cpp
    src/pipeline/ShaderModule.cpp
    src/debug/ValidationLayers.cpp
)

target_include_directories(lava PUBLIC include)
target_link_libraries(lava PUBLIC Vulkan::Vulkan)
```

### Usage in Other Projects:
```cmake
# elyrion.coregfx.renderer/CMakeLists.txt
target_link_libraries(elyrion.coregfx.renderer PRIVATE lava)
```

## Testing Strategy

### Unit Tests:
- Test each Lava component in isolation
- Mock Vulkan calls where possible
- Validate RAII cleanup behavior

### Integration Tests:
- Test Lava utilities with real Vulkan devices
- Verify interoperability with raw Vulkan API
- Performance benchmarks vs. raw Vulkan

### Validation:
- Run all tests with Vulkan validation layers enabled
- Check for memory leaks with VMA or similar tools
- Verify correct synchronization behavior

## Documentation Requirements

### API Documentation:
- Doxygen comments for all public APIs
- Usage examples for each component
- Migration guide from raw Vulkan to Lava

### Design Documentation:
- Architecture overview (this file)
- Component interaction diagrams
- Best practices and patterns

## Future Considerations

### Potential Additions:
- **Render graph abstraction** - For complex multi-pass rendering
- **Descriptor allocator** - Efficient descriptor set management
- **Texture loader** - Integration with image loading libraries
- **Compute pipeline builder** - Similar to graphics pipeline builder
- **Ray tracing utilities** - When RT features are needed

### Performance Profiling:
- Integrate with RenderDoc for frame capture
- GPU timestamp queries for performance measurement
- Memory usage tracking and reporting

## Dependencies

### Required:
- **Vulkan SDK 1.3.231.1** - Core Vulkan API
- **GLM** - Mathematics library (for debug markers, colors, etc.)

### Optional:
- **VulkanMemoryAllocator (VMA)** - Advanced memory management (future)
- **SPIRV-Reflect** - Shader reflection for descriptor set generation (future)

## Open Questions

1. **Should Lava include VMA integration?** - Pros: Better memory management. Cons: Additional dependency.
2. **Header-only vs compiled library?** - Header-only is more flexible, compiled is faster to build.
3. **Versioning strategy?** - Should Lava have its own version separate from Arctic Game Client?
4. **Namespace convention?** - `lava::` or `arctic::lava::` or `vk::` (too generic)?
5. **Error handling approach?** - Exceptions vs return codes vs asserts?

## Conclusion

**Lava** is designed to be a **lightweight, reusable Vulkan toolkit** that reduces boilerplate without hiding Vulkan's power. It focuses on developer productivity while maintaining zero-cost abstractions and explicit control.

By providing RAII wrappers, builder patterns, and common utilities, Lava enables faster Vulkan development across all Arctic Game Client projects while remaining flexible and interoperable with raw Vulkan code.

---

**Status**: Planning phase - awaiting feedback and refinement before implementation.