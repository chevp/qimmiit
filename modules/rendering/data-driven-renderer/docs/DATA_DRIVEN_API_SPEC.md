# Data-Driven Renderer API Specification

**Version:** 1.0.0
**Date:** 2025-10-03
**Architecture:** Central Java Runtime Server → Local Java Layer (JNI) → C++ Renderer

---

## Overview

The Data-Driven Renderer API is a minimal, shader-instance-focused interface between a central Java runtime server and a local C++ Vulkan renderer. The API uses **two primary data transfer objects** (ReceiveData and ResponseData) to enable server-controlled real-time rendering without entity abstractions.

**Key Principles:**
- **No entity awareness** in local Java layer or C++ renderer
- **Shader instances** as fundamental rendering units
- **Server-driven updates** via XML fragments or structured commands
- **Stateless renderer** - C++ owns rendering pipeline, Java server owns game logic

---

## Architecture Flow

```
┌─────────────────────────────────────┐
│  Central Java Runtime Server        │
│  - Entity System                    │
│  - Game Logic                       │
│  - Generates Render Commands        │
└──────────────┬──────────────────────┘
               │ (gRPC/Network)
               │ RenderCommand messages
               ↓
┌─────────────────────────────────────┐
│  Local Java Layer (JNI Bridge)      │
│  - Translates to ReceiveData        │
│  - NO entity knowledge              │
│  - Calls native methods             │
└──────────────┬──────────────────────┘
               │ (JNI)
               │ ReceiveData → ResponseData
               ↓
┌─────────────────────────────────────┐
│  C++ Data-Driven Renderer           │
│  - Vulkan Pipeline                  │
│  - Shader Instance Management       │
│  - Returns ResponseData             │
└─────────────────────────────────────┘
```

---

## Core API

### Java Bridge Interface

```java
package com.arctic.renderer.bridge;

/**
 * Minimal JNI bridge to data-driven C++ renderer
 */
public class DataDrivenRendererBridge {

    /**
     * Execute a single render command
     * @param request Command and data to send to renderer
     * @return Response with telemetry and status
     */
    public native ResponseData execute(ReceiveData request);

    /**
     * Initialize native renderer
     * @param config Renderer configuration
     */
    public native void initialize(RendererConfig config);

    /**
     * Shutdown and cleanup renderer
     */
    public native void shutdown();
}
```

---

## Data Transfer Objects

### ReceiveData

**Purpose:** Everything the renderer receives from the server (commands, updates, scene data)

```java
package com.arctic.renderer.bridge;

import java.util.Map;

public class ReceiveData {

    // ========== Command Type ==========
    public enum CommandType {
        // Scene lifecycle
        LOAD_SCENE,              // Load complete scene from XML
        CLEAR_SCENE,             // Clear all shader instances

        // Shader instance operations
        ADD_SHADER_INSTANCE,     // Add new shader instance
        REMOVE_SHADER_INSTANCE,  // Remove shader instance
        UPDATE_SHADER_INSTANCE,  // Update existing instance

        // Uniform updates
        UPDATE_UNIFORM,          // Update single uniform
        UPDATE_UNIFORMS,         // Batch update multiple uniforms

        // Transform updates
        UPDATE_TRANSFORM,        // Update instance transform

        // Camera control
        UPDATE_CAMERA,           // Update camera state

        // Lighting
        UPDATE_DIRECTIONAL_LIGHT,
        UPDATE_POINT_LIGHT,
        UPDATE_AMBIENT_LIGHT,

        // Texture updates
        UPDATE_TEXTURE,          // Update texture binding

        // Rendering
        RENDER_FRAME,            // Execute frame rendering

        // Queries
        GET_TELEMETRY,           // Request renderer telemetry
        GET_INSTANCE_COUNT       // Query shader instance count
    }

    public CommandType command;

    // ========== Scene Loading ==========
    public String sceneXmlPath;          // Path to data-driven.index.xml
    public String sceneXmlFragment;      // Inline XML fragment

    // ========== Shader Instance Operations ==========
    public String instanceId;            // Shader instance identifier
    public String templateId;            // Shader template reference
    public ShaderInstanceConfig instanceConfig;

    // ========== Uniform Updates ==========
    public String uniformName;           // Uniform identifier
    public UniformValue uniformValue;    // Single uniform value
    public Map<String, UniformValue> uniforms;  // Batch uniform updates

    // ========== Transform Updates ==========
    public Transform transform;

    // ========== Camera ==========
    public CameraState camera;

    // ========== Lighting ==========
    public String lightId;
    public DirectionalLight directionalLight;
    public PointLight pointLight;
    public AmbientLight ambientLight;

    // ========== Texture Updates ==========
    public String textureBinding;        // Binding name (e.g., "base_color_texture")
    public String textureSrc;            // Texture path/URI

    // ========== Rendering Control ==========
    public boolean waitForCompletion;    // Block until render complete
}
```

### ResponseData

**Purpose:** Everything the renderer sends back to the server (telemetry, status, events)

```java
package com.arctic.renderer.bridge;

import java.util.Map;

public class ResponseData {

    // ========== Status ==========
    public boolean success;
    public String errorMessage;
    public int errorCode;

    // ========== Render Telemetry ==========
    public long frameTimeMs;             // Frame render time
    public int drawCalls;                // Number of draw calls
    public int trianglesRendered;        // Triangle count
    public int shaderInstanceCount;      // Active shader instances
    public long totalMemoryUsedBytes;    // GPU memory usage

    // ========== Scene State ==========
    public Map<String, Object> sceneState;  // Current scene metadata

    // ========== Events (optional) ==========
    public String eventType;             // User interaction event type
    public String eventInstanceId;       // Instance that triggered event
    public Map<String, Object> eventData;

    // ========== Query Results ==========
    public Map<String, Object> queryResult;  // Generic query response
}
```

---

## Supporting Data Structures

### ShaderInstanceConfig

```java
public class ShaderInstanceConfig {
    public String templateId;                         // "pbr_standard", "ui_gradient_button"
    public Map<String, UniformValue> uniforms;        // Shader parameters
    public Map<String, TextureBinding> textures;      // Texture bindings
    public Geometry geometry;                         // Mesh or rect geometry
    public Transform transform;                       // Position/rotation/scale
    public Map<String, SpecializationConstant> specializationConstants;
}
```

### UniformValue

```java
public class UniformValue {
    public enum UniformType {
        FLOAT, VEC2, VEC3, VEC4,
        INT, IVEC2, IVEC3, IVEC4,
        BOOL, MAT4
    }

    public String name;        // "base_color_factor", "metallic_factor"
    public UniformType type;
    public Object value;       // Actual data (float, float[], int, etc.)
}
```

### Transform

```java
public class Transform {
    public Vec3 position;      // World position
    public Vec3 rotation;      // Euler angles (pitch, yaw, roll)
    public Vec3 scale;         // Scale factors
}

public class Vec3 {
    public float x, y, z;
}
```

### CameraState

```java
public class CameraState {
    public enum CameraType {
        PERSPECTIVE, ORTHOGRAPHIC
    }

    public CameraType type;
    public float fov;          // Field of view (degrees)
    public float near;         // Near clip plane
    public float far;          // Far clip plane
    public Vec3 position;
    public Vec3 rotation;
}
```

### Lighting

```java
public class DirectionalLight {
    public Vec3 direction;     // Light direction vector
    public Vec3 color;         // RGB color
    public float intensity;
}

public class PointLight {
    public Vec3 position;
    public Vec3 color;
    public float intensity;
    public float range;        // Attenuation range
}

public class AmbientLight {
    public Vec3 color;
    public float intensity;
}
```

### Geometry

```java
public class Geometry {
    public enum GeometryType {
        MESH,     // GLTF mesh reference
        RECT,     // 2D rectangle (for UI)
        SPHERE,   // Procedural sphere
        CUBE      // Procedural cube
    }

    public GeometryType type;
    public String meshSrc;     // GLTF path (if type == MESH)
    public float width;        // For RECT
    public float height;       // For RECT
}
```

### TextureBinding

```java
public class TextureBinding {
    public String bindingName;     // "base_color_texture", "normal_texture"
    public String textureSrc;      // Texture file path or URI
    public TextureSampler sampler; // Sampler configuration
}

public class TextureSampler {
    public enum FilterMode { NEAREST, LINEAR }
    public enum AddressMode { REPEAT, CLAMP_TO_EDGE, MIRRORED_REPEAT }

    public FilterMode magFilter;
    public FilterMode minFilter;
    public AddressMode addressModeU;
    public AddressMode addressModeV;
}
```

---

## XML Schema Vocabulary

The Data-Driven Renderer API uses XML as the primary scene description format. The schema vocabulary matches the API data structures exactly.

### Root Element: `<data-driven-scene>`

```xml
<data-driven-scene id="scene-id" version="1.0.0">
  <!-- Scene content -->
</data-driven-scene>
```

### Shader Templates

```xml
<shader-templates>
  <template id="pbr_standard" src="shaders/pbr-shader-template.json" />
  <template id="ui_gradient_button" src="shaders/ui_gradient_button.json" />
</shader-templates>
```

### Shader Instances

```xml
<shader-instances>
  <instance id="metal_crate_01" template="pbr_standard">
    <uniforms>
      <uniform name="base_color_factor" type="vec4" value="1.0, 1.0, 1.0, 1.0" />
      <uniform name="metallic_factor" type="float" value="1.0" />
      <uniform name="roughness_factor" type="float" value="0.5" />
    </uniforms>
    <textures>
      <texture binding="base_color_texture" src="textures/metal_albedo.png">
        <sampler mag_filter="LINEAR" min_filter="LINEAR" address_mode="REPEAT" />
      </texture>
    </textures>
    <geometry>
      <mesh src="models/crate.gltf" />
    </geometry>
    <transform>
      <position x="0.0" y="0.0" z="0.0" />
      <rotation x="0.0" y="45.0" z="0.0" />
      <scale x="1.0" y="1.0" z="1.0" />
    </transform>
  </instance>
</shader-instances>
```

### Camera

```xml
<camera>
  <type>PERSPECTIVE</type>
  <fov>60.0</fov>
  <near>0.1</near>
  <far>1000.0</far>
  <position x="0.0" y="3.0" z="15.0" />
  <rotation x="0.0" y="0.0" z="0.0" />
</camera>
```

### Lights

```xml
<lights>
  <directional-light id="sun">
    <direction x="-0.5" y="-1.0" z="-0.3" />
    <color r="1.0" g="0.95" b="0.8" />
    <intensity>3.0</intensity>
  </directional-light>

  <point-light id="lamp_01">
    <position x="5.0" y="3.0" z="0.0" />
    <color r="1.0" g="0.8" b="0.5" />
    <intensity>10.0</intensity>
    <range>15.0</range>
  </point-light>
</lights>
```

---

## Usage Examples

### Example 1: Load Scene

```java
ReceiveData request = new ReceiveData();
request.command = CommandType.LOAD_SCENE;
request.sceneXmlPath = "scenes/game_level_01.xml";

ResponseData response = rendererBridge.execute(request);
if (response.success) {
    System.out.println("Scene loaded: " + response.shaderInstanceCount + " instances");
}
```

### Example 2: Add Shader Instance

```java
ShaderInstanceConfig config = new ShaderInstanceConfig();
config.templateId = "pbr_standard";

config.uniforms = new HashMap<>();
config.uniforms.put("metallic_factor", new UniformValue("metallic_factor", UniformType.FLOAT, 0.8f));
config.uniforms.put("roughness_factor", new UniformValue("roughness_factor", UniformType.FLOAT, 0.5f));

config.transform = new Transform();
config.transform.position = new Vec3(5.0f, 2.0f, 0.0f);
config.transform.rotation = new Vec3(0.0f, 45.0f, 0.0f);
config.transform.scale = new Vec3(1.0f, 1.0f, 1.0f);

ReceiveData request = new ReceiveData();
request.command = CommandType.ADD_SHADER_INSTANCE;
request.instanceId = "dynamic_crate_01";
request.instanceConfig = config;

ResponseData response = rendererBridge.execute(request);
```

### Example 3: Update Uniform (Real-Time Material Change)

```java
ReceiveData request = new ReceiveData();
request.command = CommandType.UPDATE_UNIFORM;
request.instanceId = "smoke_stack_01";
request.uniformName = "density";
request.uniformValue = new UniformValue("density", UniformType.FLOAT, 0.85f);

rendererBridge.execute(request);
```

### Example 4: Update Camera Position

```java
CameraState camera = new CameraState();
camera.type = CameraType.PERSPECTIVE;
camera.fov = 60.0f;
camera.position = new Vec3(10.0f, 5.0f, 20.0f);
camera.rotation = new Vec3(15.0f, 30.0f, 0.0f);

ReceiveData request = new ReceiveData();
request.command = CommandType.UPDATE_CAMERA;
request.camera = camera;

rendererBridge.execute(request);
```

### Example 5: Render Frame and Get Telemetry

```java
ReceiveData request = new ReceiveData();
request.command = CommandType.RENDER_FRAME;

ResponseData response = rendererBridge.execute(request);
System.out.println("Frame time: " + response.frameTimeMs + "ms");
System.out.println("Draw calls: " + response.drawCalls);
System.out.println("Triangles: " + response.trianglesRendered);
```

---

## Server Update Flow (60Hz Example)

```java
// Central Java Runtime Server - Game Loop
while (gameRunning) {
    // 1. Update game entities (server-side logic)
    entitySystem.update(deltaTime);

    // 2. Generate shader instance updates from entity states
    for (Entity entity : entitySystem.getVisibleEntities()) {
        // Convert entity to shader instance update
        ReceiveData update = new ReceiveData();
        update.command = CommandType.UPDATE_TRANSFORM;
        update.instanceId = entity.getRendererId();
        update.transform = convertToTransform(entity.getPosition(), entity.getRotation());

        // Send to local renderer via gRPC/network
        grpcClient.send(update);
    }

    // 3. Trigger frame render
    ReceiveData renderCmd = new ReceiveData();
    renderCmd.command = CommandType.RENDER_FRAME;
    ResponseData telemetry = grpcClient.sendAndWait(renderCmd);

    // 4. Process telemetry
    logger.info("Frame: {}ms, DrawCalls: {}", telemetry.frameTimeMs, telemetry.drawCalls);

    sleep(16); // 60Hz
}
```

---

## Key Design Decisions

### Why Two Objects Only?

**Simplicity:** Single request/response pattern minimizes JNI complexity
**Flexibility:** Command enum allows extensibility without changing signatures
**Type Safety:** Java objects map cleanly to native C++ structures
**Performance:** Batch updates via maps reduce JNI crossing overhead

### Why No Entity Awareness?

**Separation of Concerns:** Entity logic belongs to game server, not renderer
**Renderer Statelessness:** C++ renderer is dumb - just renders what it's told
**Testing:** Mock shader instance updates without entity system complexity
**Scalability:** Multiple renderers can connect to one central server

### Why XML Fragments?

**Declarative:** Scene structure is self-documenting
**Server-Generated:** Central server can generate XML from entity states
**Incremental Updates:** Send small XML fragments for dynamic changes
**Tooling:** Existing XML parsers and validators

---

## Error Handling

### Response Error Codes

| Code | Meaning |
|------|---------|
| 0    | Success |
| 100  | Invalid command type |
| 200  | Shader instance not found |
| 201  | Shader template not found |
| 300  | Invalid uniform name or type |
| 400  | XML parse error |
| 500  | Vulkan rendering error |
| 600  | Resource not found (texture, mesh) |

### Example Error Response

```java
ResponseData response = rendererBridge.execute(request);
if (!response.success) {
    System.err.println("Error " + response.errorCode + ": " + response.errorMessage);
    // Handle error (e.g., retry, fallback, log)
}
```

---

## Performance Considerations

### Batch Updates

Use `UPDATE_UNIFORMS` with map instead of individual `UPDATE_UNIFORM` calls:

```java
// ❌ BAD: Multiple JNI crossings
for (String uniformName : uniformsToUpdate.keySet()) {
    ReceiveData req = new ReceiveData();
    req.command = CommandType.UPDATE_UNIFORM;
    req.uniformName = uniformName;
    req.uniformValue = uniformsToUpdate.get(uniformName);
    rendererBridge.execute(req);
}

// ✅ GOOD: Single JNI crossing
ReceiveData req = new ReceiveData();
req.command = CommandType.UPDATE_UNIFORMS;
req.instanceId = "shader_instance_01";
req.uniforms = uniformsToUpdate;
rendererBridge.execute(req);
```

### Async Rendering

For non-blocking updates:

```java
request.waitForCompletion = false;
rendererBridge.execute(request);  // Returns immediately
```

### Telemetry Throttling

Don't query telemetry every frame:

```java
if (frameCount % 60 == 0) {  // Every second at 60 FPS
    ReceiveData req = new ReceiveData();
    req.command = CommandType.GET_TELEMETRY;
    ResponseData telemetry = rendererBridge.execute(req);
}
```

---

## Version History

- **1.0.0** (2025-10-03): Initial specification
  - Two-object API design (ReceiveData/ResponseData)
  - Shader instance vocabulary
  - XML schema alignment
  - Command-based architecture

---

## References

- [data-driven.index.xml](data-driven.index.xml) - Example scene file
- [pbr-shader-template-example.json](../../../apps/desktop/penguin-on-screen/pbr-shader-template-example.json) - Shader template example
- [CLAUDE.md](../../../CLAUDE.md) - Project build and configuration guide
