# Hybrid Material Override System

## Overview

The data-driven renderer uses a **Hybrid Material System** that combines:
- **GLTF embedded materials** (defaults from 3D models)
- **gRPC real-time overrides** (server-controlled material properties)
- **OceanPbrApp rendering** (handles all Vulkan/shader complexity)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Data-Driven Renderer                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  GLTF Models           gRPC Server         Material          │
│  (embedded materials)  (shader_updates)    Override          │
│         │                     │             Manager          │
│         └─────────┬───────────┘                │             │
│                   ▼                            ▼             │
│            ┌──────────────────────────────────────┐          │
│            │     OceanPbrApp (PBR Renderer)      │          │
│            │  - Vulkan pipelines                 │          │
│            │  - Descriptor sets                  │          │
│            │  - Shader compilation               │          │
│            │  - Command buffer recording         │          │
│            └──────────────────────────────────────┘          │
│                           │                                  │
│                           ▼                                  │
│                    Rendered Frame                            │
│                (GLTF defaults + gRPC overrides)              │
└─────────────────────────────────────────────────────────────┘
```

## How It Works

### 1. Default Rendering (No Overrides)

```cpp
// GLTF model loads with embedded materials
assetLoader_.loadGltfModel("suzanne", "suzanne-in-box.gltf");

// OceanPbrApp renders with GLTF materials
pbrApp_->insertGltf3(&gltfModelContext_);
pbrApp_->render2();  // Uses embedded materials
```

**Result**: Model renders with its original materials from the GLTF file.

### 2. Server Sends Material Override

```protobuf
// Server sends gRPC message
message ShaderUpdate {
    string instance_id = "suzanne_scene_01";
    map<string, UniformValue> uniforms = {
        {"metallic_factor": 0.8},
        {"roughness_factor": 0.2},
        {"base_color_factor": [1.0, 0.5, 0.0, 1.0]}
    };
}
```

### 3. Override Storage

```cpp
void Renderer::updateShaderInstance(const rendering::ShaderUpdate& update) {
    // Store overrides in MaterialOverrideManager
    materialOverrides_.setOverride("suzanne_scene_01", "metallic_factor", 0.8f);
    materialOverrides_.setOverride("suzanne_scene_01", "roughness_factor", 0.2f);
    materialOverrides_.setOverride("suzanne_scene_01", "base_color_factor",
                                  glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
}
```

### 4. Apply Overrides Before Rendering (TODO)

```cpp
// TODO: Hook into OceanPbrApp render loop
void Renderer::renderFrame() {
    // Before rendering, apply material overrides
    applyMaterialOverrides();

    // OceanPbrApp renders with modified materials
    pbrApp_->render2();
}

void Renderer::applyMaterialOverrides() {
    // For each GLTF model/primitive
    // Check if material overrides exist
    // Apply overrides to PushConstBlockMaterial before rendering
}
```

## Supported Material Properties

### PBR Metallic-Roughness

| Property | Type | GLTF Default | gRPC Override |
|----------|------|--------------|---------------|
| `base_color_factor` | vec4 | From GLTF | ✅ Supported |
| `metallic_factor` | float | From GLTF | ✅ Supported |
| `roughness_factor` | float | From GLTF | ✅ Supported |

### Emissive

| Property | Type | GLTF Default | gRPC Override |
|----------|------|--------------|---------------|
| `emissive_factor` | vec3 | From GLTF | ✅ Supported |
| `emissive_strength` | float | 1.0 | ✅ Supported |

### Advanced

| Property | Type | GLTF Default | gRPC Override |
|----------|------|--------------|---------------|
| `alpha_mask` | float | From GLTF | ✅ Supported |
| `alpha_mask_cutoff` | float | From GLTF | ✅ Supported |
| `normal_scale` | float | 1.0 | ✅ Supported |
| `occlusion_strength` | float | 1.0 | ✅ Supported |

### Transmission (KHR_materials_transmission)

| Property | Type | GLTF Default | gRPC Override |
|----------|------|--------------|---------------|
| `transmission_factor` | float | 0.0 | ✅ Supported |
| `ior` | float | 1.5 | ✅ Supported |

## Example Usage

### XML Configuration (data-driven.index.xml)

```xml
<instance id="suzanne_scene_01" template="pbr_standard">
    <uniforms>
        <!-- Initial values (can be overridden by gRPC) -->
        <uniform name="base_color_factor" type="vec4" value="1.0, 1.0, 1.0, 1.0" />
        <uniform name="metallic_factor" type="float" value="0.0" />
        <uniform name="roughness_factor" type="float" value="0.5" />
    </uniforms>
    <geometry>
        <mesh src="asset://models/suzanne-in-box/SuzanneInBox.gltf" />
    </geometry>
</instance>
```

### gRPC Server Update (Java/Quarkus)

```java
// Send material update to renderer
ShaderUpdate update = ShaderUpdate.newBuilder()
    .setInstanceId("suzanne_scene_01")
    .putUniforms("metallic_factor",
        UniformValue.newBuilder().setFloatValue(0.9f).build())
    .putUniforms("roughness_factor",
        UniformValue.newBuilder().setFloatValue(0.1f).build())
    .putUniforms("base_color_factor",
        UniformValue.newBuilder()
            .setVec4Value(Vec4.newBuilder()
                .setX(1.0f).setY(0.0f).setZ(0.0f).setW(1.0f))
            .build())
    .build();

rendererClient.updateShader(update);
```

### Result

- **Suzanne** renders with:
  - **Geometry**: From suzanne-in-box.gltf
  - **Textures**: From GLTF embedded textures
  - **Base Color**: Red (1.0, 0.0, 0.0, 1.0) ← gRPC override
  - **Metallic**: 0.9 ← gRPC override
  - **Roughness**: 0.1 ← gRPC override

## Benefits of Hybrid Approach

### ✅ Advantages

1. **No Shader Compilation Needed**
   - OceanPbrApp handles all shader complexity
   - No SPIR-V files to manage
   - No Vulkan pipeline creation code

2. **GLTF Models "Just Work"**
   - Drop in any GLTF model with embedded materials
   - Renders correctly immediately
   - No material conversion needed

3. **Server Has Full Control**
   - Override any material property via gRPC
   - Real-time updates (60Hz+)
   - No client-side code changes

4. **Simplicity**
   - MaterialOverrideManager is ~100 lines of code
   - No complex shader template system
   - Easy to understand and maintain

### ❌ Limitations

1. **PBR Only**
   - Can't create custom volumetric effects
   - Can't create custom UI shaders
   - Limited to PBR material model

2. **Override Granularity**
   - Overrides apply to entire shader instance
   - Can't override per-mesh or per-primitive (yet)

3. **No Custom Shaders**
   - Stuck with OceanPbrApp's PBR implementation
   - Can't add custom lighting models

## JSON Template Role

In the hybrid approach, JSON templates (pbr_glass.json, etc.) serve as **documentation only**:

```json
{
  "template_id": "pbr_standard",
  "description": "Standard PBR shader - uses GLTF embedded materials with gRPC overrides",

  "uniforms": [
    {
      "name": "metallic_factor",
      "type": "float",
      "default": "From GLTF",
      "grpc_override": "supported"
    }
  ]
}
```

They describe:
- ✅ What properties can be overridden
- ✅ Valid ranges and types
- ✅ Default behavior

They do NOT:
- ❌ Reference actual shader files (.vert.spv, .frag.spv)
- ❌ Create Vulkan pipelines
- ❌ Load SPIR-V bytecode

## Future: Custom Shaders

If you later want **custom shaders beyond PBR**, you can extend the system:

1. **Keep MaterialOverrideManager** for GLTF models (hybrid)
2. **Add Custom Pipeline System** for special effects:
   ```cpp
   if (instance.hasCustomShader()) {
       // Use JSON template + SPIR-V shaders
       renderWithCustomPipeline(instance);
   } else {
       // Use OceanPbrApp + material overrides
       renderWithPbrApp(instance);
   }
   ```

## Implementation Status

- ✅ MaterialOverride class
- ✅ MaterialOverrideManager
- ✅ gRPC integration (Renderer::updateShaderInstance)
- ✅ Override storage
- ⏳ **TODO**: Apply overrides before rendering
- ⏳ **TODO**: Map instance ID to GLTF primitives
- ⏳ **TODO**: Hook into OceanPbrApp render loop

## Next Steps

1. Map shader instance IDs to GLTF model/primitive indices
2. Hook into OceanPbrApp before push constant update
3. Apply MaterialOverride values to PushConstBlockMaterial
4. Test with gRPC updates

---

**This hybrid approach gives you data-driven control without reimplementing a rendering engine.**
