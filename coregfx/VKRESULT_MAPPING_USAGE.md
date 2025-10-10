# VkResult to CryoErrorCode Mapping - Usage Guide

This document explains how to use the VkResult → CryoErrorCode mapping system for comprehensive Vulkan error tracking.

## Features

1. **VkResult to String Conversion** - Convert numeric VkResult codes to human-readable names
2. **CryoErrorCode Integration** - Map Vulkan errors to standardized Cryo error codes
3. **ErrorReport Proto Message** - Send detailed error reports to server via gRPC
4. **Cross-Language Support** - Error codes available in TypeScript, Python, C++ via protobuf

## Basic Usage

### 1. Simple Error Logging with VkResult Names

```cpp
#include <coregfx/core/CryoErrorCodes.hpp>
#include <coregfx/core/ocean_log.hpp>

VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &fence);
if (result != VK_SUCCESS) {
    // Set Cryo error code
    cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_FENCE_FAILED);

    // Log with VkResult name
    ocean::error("Fence creation failed. VkResult: " + std::to_string(result) +
                " (" + std::string(cryo::vkResultToString(result)) + ")");

    // Example output:
    // ERROR: Fence creation failed. VkResult: -1 (VK_ERROR_OUT_OF_HOST_MEMORY)
}
```

### 2. Create Full ErrorReport for gRPC

```cpp
#include <coregfx/core/CryoErrorCodes.hpp>

VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore);
if (result != VK_SUCCESS) {
    // Create comprehensive error report
    cgfx::ErrorReport report = cryo::createErrorReport(
        cgfx::CRYO_ERROR_VULKAN_SEMAPHORE_FAILED,
        result,
        "Failed to create synchronization semaphore for swapchain",
        "elyrion"  // renderer type
    );

    // Send to server via gRPC
    // grpcClient->reportError(report);

    // Or log locally
    ocean::error("ErrorReport: " + report.DebugString());
}
```

### 3. ErrorReport Proto Structure

The `ErrorReport` message contains:

```protobuf
message ErrorReport {
    CryoErrorCode code = 1;        // CRYO_ERROR_VULKAN_FENCE_FAILED
    string message = 2;             // "Vulkan fence creation failed"
    string details = 3;             // Custom details string
    int64 timestamp = 4;            // Unix timestamp
    string renderer_id = 5;         // "elyrion", "data-driven", etc.
    int32 vk_result = 6;            // -1 (numeric VkResult)
    string vk_result_name = 7;      // "VK_ERROR_OUT_OF_HOST_MEMORY"
}
```

### 4. Example ErrorReport JSON Output

```json
{
  "code": "CRYO_ERROR_VULKAN_FENCE_FAILED",
  "message": "Vulkan fence creation failed",
  "details": "Failed to create fence 0 during swapchain initialization",
  "timestamp": 1704567890,
  "renderer_id": "elyrion",
  "vk_result": -1,
  "vk_result_name": "VK_ERROR_OUT_OF_HOST_MEMORY"
}
```

## Complete Example: Vulkan Pipeline Creation

```cpp
#include <coregfx/core/CryoErrorCodes.hpp>
#include <coregfx/core/ocean_log.hpp>

bool createGraphicsPipeline(VkDevice device, VkPipeline* pipeline) {
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    // ... setup pipeline info ...

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline);

    if (result != VK_SUCCESS) {
        // Set global error code
        cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_PIPELINE_FAILED);

        // Create detailed error report
        cgfx::ErrorReport report = cryo::createErrorReport(
            cgfx::CRYO_ERROR_VULKAN_PIPELINE_FAILED,
            result,
            "Graphics pipeline creation failed for PBR shader",
            "elyrion"
        );

        // Log detailed error
        ocean::error(
            "Pipeline creation failed!\n" +
            std::string("  Code: ") + cryo::cryoErrorCodeToString(report.code()) + "\n" +
            std::string("  VkResult: ") + report.vk_result_name() + " (" + std::to_string(report.vk_result()) + ")\n" +
            std::string("  Details: ") + report.details()
        );

        // Send to server (optional)
        // grpcClient->reportError(report);

        return false;
    }

    return true;
}
```

## Supported VkResult Codes

The system supports all standard VkResult codes including:

**Success Codes:**
- `VK_SUCCESS` (0)
- `VK_NOT_READY`, `VK_TIMEOUT`, `VK_EVENT_SET`, etc.

**Error Codes:**
- `VK_ERROR_OUT_OF_HOST_MEMORY` (-1)
- `VK_ERROR_OUT_OF_DEVICE_MEMORY` (-2)
- `VK_ERROR_INITIALIZATION_FAILED` (-3)
- `VK_ERROR_DEVICE_LOST` (-4)
- `VK_ERROR_OUT_OF_DATE_KHR` (-1000001004)
- And 30+ more Vulkan error codes

**Unknown Codes:**
- Returns `"VK_UNKNOWN_RESULT"` for unrecognized codes

## Integration with Logging

### Standard Logging (Current Implementation)

```cpp
VkResult result = createFenceInternal(device, &fence, signaled);
if (result != VK_SUCCESS) {
    cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_FENCE_FAILED);

    ocean::error("Failed to create fence " + std::to_string(i) +
                ". VkResult: " + std::to_string(result) +
                " (" + std::string(cryo::vkResultToString(result)) + ")");
}

// Output: ERROR: Failed to create fence 0. VkResult: -1 (VK_ERROR_OUT_OF_HOST_MEMORY)
```

### Enhanced Logging with ErrorReport

```cpp
VkResult result = createFenceInternal(device, &fence, signaled);
if (result != VK_SUCCESS) {
    auto report = cryo::createErrorReport(
        cgfx::CRYO_ERROR_VULKAN_FENCE_FAILED,
        result,
        "Fence " + std::to_string(i) + " creation failed during initialization",
        "coregfx"
    );

    ocean::error(report.DebugString());
    // Optionally send to telemetry server
}
```

## Server-Side Usage (TypeScript/Node.js)

Once proto files are compiled for TypeScript, error reports can be received on the server:

```typescript
import { CryoErrorCode, ErrorReport } from './generated/cgfx_pb';

function handleRendererError(report: ErrorReport) {
    console.error(`Renderer Error from ${report.renderer_id}:`);
    console.error(`  Code: ${CryoErrorCode[report.code]}`);
    console.error(`  Message: ${report.message}`);
    console.error(`  VkResult: ${report.vk_result_name} (${report.vk_result})`);
    console.error(`  Details: ${report.details}`);
    console.error(`  Timestamp: ${new Date(report.timestamp * 1000).toISOString()}`);

    // Store in database, send alerts, etc.
}
```

## Benefits

1. **Unified Error Tracking** - CryoErrorCode + VkResult in one place
2. **Human-Readable Logs** - No need to look up VkResult codes manually
3. **Server Monitoring** - Send error reports to server for analytics
4. **Cross-Language** - Error codes available in all protobuf-supported languages
5. **Debugging** - Detailed error context with timestamps and renderer info
6. **Telemetry** - Track which VkResult errors occur most frequently

## See Also

- [coregfx/include/coregfx/core/CryoErrorCodes.hpp](coregfx/include/coregfx/core/CryoErrorCodes.hpp) - Full API
- [arctic-proto/cgfx.proto](arctic-proto/cgfx.proto) - Proto definitions
- [coregfx/src/util/synchronization_manager.cpp](coregfx/src/util/synchronization_manager.cpp) - Real-world usage
