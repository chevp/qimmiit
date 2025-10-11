# Shader Templates - Hybrid Rendering System

## Purpose

These JSON files define shader capabilities for the **Hybrid Rendering System**:
- **GLTF Models**: Use OceanPbrApp's built-in PBR shaders + material overrides via gRPC
- **Custom Effects**: Load custom SPIR-V shaders, create Vulkan pipelines from JSON templates

## Why These Exist

The data-driven renderer uses **two rendering paths**:

### Path 1: GLTF Models (PBR via OceanPbrApp)
```
GLTF Model → OceanPbrApp (built-in PBR shaders) → Material Overrides via gRPC
```
- ✅ Uses OceanPbrApp's built-in PBR shaders
- ✅ Applies material overrides from MaterialOverrideManager
- ❌ No custom shader loading needed
- ❌ No Vulkan pipeline creation needed

**Templates**: pbr_standard.json, pbr_glass.json, pbr_emissive.json

### Path 2: Custom Effects (Custom Shaders)
```
JSON Template → Load SPIR-V → Create VkPipeline → Render with Custom Shader
```
- ✅ Loads SPIR-V shaders referenced in JSON
- ✅ Creates Vulkan pipelines from JSON metadata
- ✅ Manages custom descriptor sets
- ✅ Full shader control for non-PBR effects

**Templates**: volumetric_smoke.json, ui_gradient_button.json, ui_panel.json

## What They Actually Do

### ✅ Documentation
```json
{
  "template_id": "pbr_glass",
  "uniforms": [
    {
      "name": "transmission_factor",
      "type": "float",
      "default": 0.95,
      "description": "Glass transparency (0=opaque, 1=fully transparent)"
    }
  ]
}
```

This tells developers:
- What properties exist
- Valid types and ranges
- Default values
- Human-readable descriptions

### ✅ API Contract
The JSON defines what the server can override via gRPC:

```java
// Server knows it can send this because pbr_glass.json documents it
ShaderUpdate.newBuilder()
    .setInstanceId("window_panel_01")
    .putUniforms("transmission_factor",
        UniformValue.newBuilder().setFloatValue(0.8f).build())
    .build();
```

### ✅ Custom Shader Support
For non-PBR effects (smoke, UI, etc.), templates reference actual SPIR-V shaders:

```json
{
  "shaders": {
    "vertex": "shaders/volumetric.vert.spv",
    "fragment": "shaders/volumetric_smoke.frag.spv"
  }
}
```

ShaderManager loads these SPIR-V files and creates custom Vulkan pipelines.

## How the Hybrid System Works

### Path 1: GLTF Models (PBR with Material Overrides)

```
┌─────────────────────────────────────────────────────────────┐
│         PBR Template JSON (pbr_glass.json)                  │
│         Documentation only - OceanPbrApp has shaders        │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              data-driven.index.xml                           │
│                                                              │
│  <instance id="window_panel_01" template="pbr_glass">       │
│      <geometry>                                              │
│          <mesh src="asset://models/window.gltf" />          │
│      </geometry>                                             │
│  </instance>                                                 │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              MaterialOverrideManager                         │
│  Stores gRPC overrides: { "window_panel_01": { ior: 1.5 } }│
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                   OceanPbrApp                                │
│  Built-in PBR shaders + GLTF geometry + Material Overrides  │
└─────────────────────────────────────────────────────────────┘
```

### Path 2: Custom Effects (Custom Shaders & Pipelines)

```
┌─────────────────────────────────────────────────────────────┐
│    Custom Template JSON (volumetric_smoke.json)             │
│    References SPIR-V shaders: volumetric_smoke.frag.spv     │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              data-driven.index.xml                           │
│                                                              │
│  <instance id="smoke_stack_01" template="volumetric_smoke"> │
│      <uniforms>                                              │
│          <uniform name="density" type="float" value="0.7"/> │
│      </uniforms>                                             │
│  </instance>                                                 │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              ShaderManager                                   │
│  1. Load volumetric_smoke.frag.spv                          │
│  2. Create VkPipeline from JSON metadata                    │
│  3. Allocate descriptor sets for uniforms                   │
│  4. Record draw commands                                     │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│            Custom Vulkan Pipeline Rendering                  │
│  Ray-marched volumetric smoke with custom fragment shader   │
└─────────────────────────────────────────────────────────────┘
```

## Template Breakdown

### [pbr_standard.json](../../shaders/templates/pbr_standard.json)

**Standard PBR Metallic-Roughness Workflow**

Maps to `PushConstBlockMaterial` in OceanPbrApp:

| JSON Property | C++ Field | GLTF Default | gRPC Override |
|---------------|-----------|--------------|---------------|
| `base_color_factor` | `baseColorFactor` | From GLTF | ✅ |
| `metallic_factor` | `metallicFactor` | From GLTF | ✅ |
| `roughness_factor` | `roughnessFactor` | From GLTF | ✅ |
| `normal_scale` | *(not in push const)* | 1.0 | ⏳ Future |
| `occlusion_strength` | *(not in push const)* | 1.0 | ⏳ Future |

**Usage**: Standard PBR materials (wood, metal, plastic, fabric)

---

### [pbr_glass.json](pbr_glass.json)

**Transmission/Refraction (Glass, Water, etc.)**

Extends PBR with transparency:

| JSON Property | Description | Default | Range |
|---------------|-------------|---------|-------|
| `transmission_factor` | How much light passes through | 0.95 | 0-1 |
| `ior` | Index of refraction | 1.5 | 1.0-2.5 |
| `roughness_factor` | Surface roughness | 0.1 | 0-1 |

**Usage**: Windows, lenses, water surfaces, transparent materials

**Example**:
```xml
<instance id="glass_window" template="pbr_glass">
    <uniforms>
        <uniform name="transmission_factor" type="float" value="0.95" />
        <uniform name="ior" type="float" value="1.5" />  <!-- Glass = 1.5 -->
        <uniform name="roughness_factor" type="float" value="0.05" />
    </uniforms>
</instance>
```

---

### [pbr_emissive.json](pbr_emissive.json)

**Self-Illuminated Materials (Lights, Screens, etc.)**

Maps to `PushConstBlockMaterial.emissiveFactor`:

| JSON Property | C++ Field | Description |
|---------------|-----------|-------------|
| `emissive_factor` | `emissiveFactor` | RGB color of emitted light |
| `emissive_strength` | *(multiplied in shader)* | Intensity multiplier |

**Usage**: Light panels, LED screens, glowing objects, neon signs

**Example**:
```xml
<instance id="ceiling_light" template="pbr_emissive">
    <uniforms>
        <uniform name="emissive_factor" type="vec3" value="1.0, 0.8, 0.5" />
        <uniform name="emissive_strength" type="float" value="3.0" />
    </uniforms>
</instance>
```

---

### [volumetric_smoke.json](volumetric_smoke.json)

**Ray-Marched Volumetric Effects**

✅ **USES CUSTOM SHADERS** - Not a GLTF model, requires Path 2 rendering

**Implementation Requirements**:
- Custom vertex/fragment shaders (`.vert.spv`, `.frag.spv`)
- Custom Vulkan pipeline created from JSON
- 3D noise textures
- Ray-marching in fragment shader
- ShaderManager creates VkPipeline from template

**Rendering Path**: Custom shader pipeline (NOT OceanPbrApp)

**Usage**:
```xml
<instance id="smoke_stack_01" template="volumetric_smoke">
    <uniforms>
        <uniform name="density" type="float" value="0.7" />
        <uniform name="wind_direction" type="vec3" value="0.5, 0.1, 0.3" />
    </uniforms>
    <textures>
        <texture binding="noise_texture" src="textures/noise_3d.png" />
    </textures>
</instance>
```

**Current Status**: Template ready, ShaderManager needs custom pipeline creation (Phase 4)

---

### [ui_gradient_button.json](ui_gradient_button.json)

**ImGui-Style UI Shaders**

✅ **USES CUSTOM SHADERS** - 2D screen-space UI rendering with Path 2

**Implementation Requirements**:
- Custom vertex/fragment shaders for 2D quads
- Custom Vulkan pipeline with alpha blending
- Screen-space coordinate system
- Rounded rectangle SDF (Signed Distance Field) in fragment shader

**Rendering Path**: Custom UI pipeline (NOT OceanPbrApp)

**Current Status**: Template ready, ShaderManager needs 2D UI pipeline (Phase 5)

---

### [ui_panel.json](ui_panel.json)

**ImGui Panel Backgrounds**

✅ **USES CUSTOM SHADERS** - Same as ui_gradient_button.json

**Current Status**: Template ready, ShaderManager needs 2D UI pipeline (Phase 5)

## What You Can Actually Override Right Now

Using the **Hybrid Material System**, you can override these properties on **any GLTF model**:

### Metallic-Roughness PBR
```java
// Make suzanne look like shiny metal
ShaderUpdate.newBuilder()
    .setInstanceId("suzanne_scene_01")
    .putUniforms("metallic_factor", floatValue(1.0f))
    .putUniforms("roughness_factor", floatValue(0.1f))
    .putUniforms("base_color_factor", vec4(0.8f, 0.8f, 0.9f, 1.0f))
    .build();
```

### Emissive Glow
```java
// Make damaged helmet glow red
ShaderUpdate.newBuilder()
    .setInstanceId("damaged_helmet_01")
    .putUniforms("emissive_factor", vec3(1.0f, 0.0f, 0.0f))
    .putUniforms("emissive_strength", floatValue(5.0f))
    .build();
```

### Transparency (Future - requires OceanPbrApp extension)
```java
// Make barrel semi-transparent
ShaderUpdate.newBuilder()
    .setInstanceId("stylized_barrel_01")
    .putUniforms("transmission_factor", floatValue(0.5f))
    .putUniforms("ior", floatValue(1.3f))
    .build();
```

## File Organization

```
shaders/templates/
├── README.md                        ← You are here
├── pbr_glass.json                   ← Glass/transmission properties
├── pbr_emissive.json                ← Emissive/glow properties
├── volumetric_smoke.json            ← Future: volumetric effects
├── ui_gradient_button.json          ← Future: UI rendering
└── ui_panel.json                    ← Future: UI rendering

shaders/templates/
└── pbr_standard.json ← Standard PBR properties
```

## Misconceptions to Avoid

### ❌ WRONG: "JSON templates compile shaders for ALL effects"
**Reality**:
- **GLTF Models**: OceanPbrApp has built-in PBR shaders, JSON documents properties
- **Custom Effects**: ShaderManager DOES load SPIR-V and create pipelines from JSON

### ❌ WRONG: "Need pbr_glass.hpp C++ class for each shader type"
**Reality**: One MaterialOverrideManager handles ALL PBR material overrides generically

### ❌ WRONG: "Never need .vert.spv and .frag.spv files"
**Reality**:
- **GLTF Models**: No custom shaders needed (OceanPbrApp)
- **Custom Effects**: DO need SPIR-V shaders (volumetric smoke, UI, etc.)

### ❌ WRONG: "JSON never creates Vulkan pipelines"
**Reality**:
- **GLTF Models**: OceanPbrApp creates PBR pipelines internally
- **Custom Effects**: ShaderManager creates custom pipelines from JSON templates

### ✅ CORRECT: "Hybrid approach = Two rendering paths"
**Reality**:
- **Path 1**: GLTF + OceanPbrApp + Material Overrides
- **Path 2**: Custom Shaders + ShaderManager + Custom Pipelines

### ✅ CORRECT: "JSON describes both documentation AND pipeline creation"
**Reality**: Serves dual purpose depending on shader type (PBR vs Custom)

## Adding New Properties

To add a new override property:

### 1. Add to MaterialOverride.hpp
```cpp
struct MaterialOverride {
    std::optional<float> newProperty;
};
```

### 2. Add setter to MaterialOverride.cpp
```cpp
void MaterialOverrideManager::setOverride(
    const std::string& instanceId,
    const std::string& propertyName,
    float value) {

    if (propertyName == "new_property") {
        override.newProperty = value;
    }
}
```

### 3. Document in JSON
```json
{
  "uniforms": [
    {
      "name": "new_property",
      "type": "float",
      "default": 1.0,
      "description": "What this property does"
    }
  ]
}
```

### 4. Apply in render loop (TODO)
```cpp
void Renderer::applyMaterialOverrides() {
    if (override->newProperty.has_value()) {
        pushConstMaterial.newField = override->newProperty.value();
    }
}
```

**No shader recompilation needed!** Just override existing PBR parameters.

## Summary

These JSON templates are **documentation for developers and the server**.

- They describe the **gRPC API** for material overrides
- They do **NOT** affect rendering directly
- OceanPbrApp handles all shader/rendering complexity
- MaterialOverrideManager stores override values generically
- Server controls materials via gRPC in real-time

**For more details, see**: [HYBRID_MATERIAL_SYSTEM.md](../../docs/HYBRID_MATERIAL_SYSTEM.md)
