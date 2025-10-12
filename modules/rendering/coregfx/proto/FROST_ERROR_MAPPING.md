# Frost API Error Mapping

## Purpose
This document maps `cgfx.CryoErrorCode` (internal server errors) to `frost.FrostError` (client-facing errors).

**Important**: `frostapi.proto` is self-contained and does NOT import `cgfx.proto` to avoid exposing Vulkan types to clients.

## Server-Side Implementation

```cpp
// frost_service_impl.cpp
#include "frostapi.pb.h"
#include "cgfx.pb.h"  // Only server sees this

namespace {

frost::FrostError MapCgfxToFrostError(cgfx::CryoErrorCode cgfx_code) {
  switch (cgfx_code) {
    // Success
    case cgfx::CRYO_SUCCESS:
      return frost::SUCCESS;

    // Initialization Errors
    case cgfx::CRYO_ERROR_RENDERER_INIT_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_INSTANCE_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_DEVICE_FAILED:
      return frost::INITIALIZATION_FAILED;

    case cgfx::CRYO_ERROR_WINDOW_CREATION_FAILED:
      return frost::WINDOW_CREATION_FAILED;

    case cgfx::CRYO_ERROR_VULKAN_SWAPCHAIN_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_RENDERPASS_FAILED:
      return frost::GRAPHICS_DEVICE_FAILED;

    // Scene Errors
    case cgfx::CRYO_ERROR_SCENE_LOAD_FAILED:
      return frost::SCENE_LOAD_FAILED;

    case cgfx::CRYO_ERROR_SCENE_VALIDATION_FAILED:
    case cgfx::CRYO_ERROR_CONFIG_PARSE_ERROR:
      return frost::SCENE_PARSE_FAILED;

    // Asset Errors
    case cgfx::CRYO_ERROR_GLTF_LOAD_FAILED:
    case cgfx::CRYO_ERROR_ASSET_PATH_NOT_FOUND:
      return frost::ASSET_NOT_FOUND;

    case cgfx::CRYO_ERROR_GLTF_PARSE_FAILED:
      return frost::GLTF_PARSE_FAILED;

    case cgfx::CRYO_ERROR_TEXTURE_FAILED:
    case cgfx::CRYO_ERROR_ENVIRONMENT_MAP_FAILED:
    case cgfx::CRYO_ERROR_BRDF_LUT_FAILED:
    case cgfx::CRYO_ERROR_CUBEMAP_FAILED:
      return frost::TEXTURE_LOAD_FAILED;

    // Rendering Errors
    case cgfx::CRYO_ERROR_SHADER_COMPILATION_FAILED:
    case cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR:
      return frost::RENDER_FAILED;

    // Resource Errors
    case cgfx::CRYO_ERROR_OUT_OF_MEMORY:
      return frost::OUT_OF_MEMORY;

    case cgfx::CRYO_ERROR_GPU_DEVICE_LOST:
      return frost::GPU_DEVICE_LOST;

    case cgfx::CRYO_ERROR_VULKAN_FENCE_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_SEMAPHORE_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_COMMAND_BUFFER_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_DESCRIPTOR_POOL_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_DESCRIPTOR_SET_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_PIPELINE_FAILED:
    case cgfx::CRYO_ERROR_VULKAN_BUFFER_FAILED:
      return frost::RESOURCE_CREATION_FAILED;

    // gRPC/Network Errors
    case cgfx::CRYO_ERROR_GRPC_CONNECTION_FAILED:
      return frost::CONNECTION_FAILED;

    case cgfx::CRYO_ERROR_GRPC_PROTOCOL_ERROR:
      return frost::PROTOCOL_ERROR;

    // Generic Fallbacks
    case cgfx::CRYO_ERROR_RUNTIME_EXCEPTION:
    case cgfx::CRYO_ERROR_UNKNOWN_EXCEPTION:
    case cgfx::CRYO_ERROR_FATAL_CRASH:
    case cgfx::CRYO_ERROR_GENERIC_FAILURE:
    default:
      return frost::UNKNOWN_ERROR;
  }
}

} // anonymous namespace

// Example usage in Frost service implementation
grpc::Status FrostRendererImpl::LoadScene(
    grpc::ServerContext* context,
    const frost::LoadSceneRequest* request,
    frost::FrostResponse* response) {

  // Call internal coregfx renderer
  cgfx::CryoErrorCode internal_error = renderer_->LoadSceneInternal(request->file_path());

  // Map to client-facing Frost error
  response->set_success(internal_error == cgfx::CRYO_SUCCESS);
  response->set_error(MapCgfxToFrostError(internal_error));

  if (internal_error != cgfx::CRYO_SUCCESS) {
    response->set_message(GetErrorMessage(internal_error));
  }

  return grpc::Status::OK;
}
```

## Error Code Mapping Table

| cgfx::CryoErrorCode | frost::FrostError | Category |
|---------------------|-------------------|----------|
| CRYO_SUCCESS | SUCCESS | Success |
| CRYO_ERROR_RENDERER_INIT_FAILED | INITIALIZATION_FAILED | Init |
| CRYO_ERROR_VULKAN_INSTANCE_FAILED | INITIALIZATION_FAILED | Init |
| CRYO_ERROR_VULKAN_DEVICE_FAILED | INITIALIZATION_FAILED | Init |
| CRYO_ERROR_WINDOW_CREATION_FAILED | WINDOW_CREATION_FAILED | Init |
| CRYO_ERROR_VULKAN_SWAPCHAIN_FAILED | GRAPHICS_DEVICE_FAILED | Init |
| CRYO_ERROR_SCENE_LOAD_FAILED | SCENE_LOAD_FAILED | Scene |
| CRYO_ERROR_GLTF_LOAD_FAILED | ASSET_NOT_FOUND | Asset |
| CRYO_ERROR_GLTF_PARSE_FAILED | GLTF_PARSE_FAILED | Asset |
| CRYO_ERROR_TEXTURE_FAILED | TEXTURE_LOAD_FAILED | Asset |
| CRYO_ERROR_OUT_OF_MEMORY | OUT_OF_MEMORY | Resource |
| CRYO_ERROR_GPU_DEVICE_LOST | GPU_DEVICE_LOST | Resource |
| CRYO_ERROR_VULKAN_BUFFER_FAILED | RESOURCE_CREATION_FAILED | Resource |
| CRYO_ERROR_GRPC_CONNECTION_FAILED | CONNECTION_FAILED | Network |

## Client-Side Benefits

**Penguin Game Client** only needs:
```cpp
#include "frostapi.pb.h"  // ✅ Clean API - no Vulkan types
#include "frostapi.grpc.pb.h"

// NO NEED FOR:
// ❌ #include "cgfx.pb.h"
// ❌ #include <vulkan/vulkan.h>
// ❌ #include <glm/glm.hpp>
```

**Result**: Fast compilation, clean API, no graphics backend exposure.
