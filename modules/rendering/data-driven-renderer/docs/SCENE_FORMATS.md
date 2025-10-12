# Scene Format Comparison

## Overview

This project uses **two different scene formats** for different architectural layers:

| Format | File | Layer | Purpose |
|--------|------|-------|---------|
| **Entity Scene** | `java-entity-scene.elyrion.xml` | Java (Android/Quarkus) | Entity management, game logic, distributed scenes |
| **Shader Scene** | `coregfx-shader-scene.xml` | C++ (CoreGFX Renderer) | Direct shader control, GPU state, real-time rendering |

## When to Use Each Format

### 1. Entity Scene (`java-entity-scene.elyrion.xml`)

**Use this format when:**
- ✅ Working with Java layer (Android/Quarkus)
- ✅ Managing game entities and business logic
- ✅ Loading scenes from multiple servers (distributed)
- ✅ Static scenes that work offline
- ✅ Entity-based architecture (ECS pattern)
- ✅ Backend communicates via gRPC with entities

**Format characteristics:**
```xml
<elyrion id="game-scene">
  <scenes>
    <scene id="main">
      <entities>
        <entity id="player" asset="model-id" />
      </entities>
    </scene>
  </scenes>

  <assets>
    <asset id="model-id" src="models/player.gltf" />
  </assets>

  <!-- Optional: Distributed scene loading -->
  <remote>
    <url>http://server-b.com/scene.xml</url>
  </remote>

  <!-- Optional: Real-time updates -->
  <streams>
    <stream type="rpc" src="grpc://backend:9090">
      <instruction action="rpc://update-component" />
    </stream>
  </streams>
</elyrion>
```

**Data Flow:**
```
Backend (gRPC) → Java Entity Manager → Update Entities → JNI → C++ Renderer
```

### 2. Shader Scene (`coregfx-shader-scene.xml`)

**Use this format when:**
- ✅ Direct shader instance control from server
- ✅ Real-time visual effects (smoke, particles, UI)
- ✅ Server-driven GPU state updates
- ✅ Always-online rendering applications
- ✅ Custom shader parameter streaming

**Format characteristics:**
```xml
<data-driven-scene id="shader-scene">
  <backend>
    <grpc>
      <address>127.0.0.1:9081</address>
      <service>rendering.RendererService</service>
    </grpc>
  </backend>

  <shader-templates>
    <template id="pbr_standard" src="shaders/pbr.json" />
  </shader-templates>

  <shader-instances>
    <instance id="metal_crate" template="pbr_standard">
      <uniforms>
        <uniform name="roughness" type="float" value="0.5" />
      </uniforms>
      <geometry>
        <mesh src="models/crate.gltf" />
      </geometry>
    </instance>
  </shader-instances>

  <update-rules>
    <rule instance="metal_crate" stream="grpc" rate="60hz">
      <update-uniform name="roughness" />
    </rule>
  </update-rules>
</data-driven-scene>
```

**Data Flow:**
```
Backend (gRPC) → Shader Updates → C++ Renderer → Direct GPU State
```

## Recommended Architecture for This Project

### Current Setup (Java + C++ via JNI)

**Use `java-entity-scene.elyrion.xml`** because:
1. Java layer manages entities
2. Backend sends entity updates via gRPC to Java
3. Java updates C++ renderer via JNI with transform data
4. C++ only renders what Java tells it to render

```
┌─────────────────────────────────────┐
│ Backend (gRPC)                      │
│ - Entity state                      │
│ - Game logic                        │
└────────────┬────────────────────────┘
             │ gRPC (entities)
             ↓
┌─────────────────────────────────────┐
│ Java Client                         │
│ - Loads: java-entity-scene.elyrion.xml
│ - Entity management                 │
│ - Business logic                    │
└────────────┬────────────────────────┘
             │ JNI (transforms)
             ↓
┌─────────────────────────────────────┐
│ C++ Data-Driven Renderer            │
│ - Receives: entity transforms       │
│ - Renders: Vulkan/PBR               │
└─────────────────────────────────────┘
```

### Special Use Case: Direct Shader Control

**Use `coregfx-shader-scene.xml`** only if:
- Backend needs to control shaders directly (not entities)
- Real-time visual effects without entity abstraction
- Server-side rendering control

```
┌─────────────────────────────────────┐
│ Rendering Service (gRPC)            │
│ - Shader parameter updates          │
│ - Direct GPU state control          │
└────────────┬────────────────────────┘
             │ gRPC (shader updates)
             ↓
┌─────────────────────────────────────┐
│ C++ Shader-Driven Renderer          │
│ - Loads: coregfx-shader-scene.xml  │
│ - Direct shader instance control    │
│ - No entity abstraction             │
└─────────────────────────────────────┘
```

## File Naming Convention

| Original Name | New Name | Purpose |
|---------------|----------|---------|
| `index.elyrion.xml` | `java-entity-scene.elyrion.xml` | Java layer entity scene |
| `data-driven.index.xml` | `coregfx-shader-scene.xml` | C++ shader-driven scene |

## Comparison Table

| Feature | Entity Scene | Shader Scene |
|---------|-------------|--------------|
| **Layer** | Java (JNI → C++) | C++ only |
| **Abstraction** | Entities, Components | Shader instances |
| **Backend Protocol** | Optional gRPC (entities) | Required gRPC (shader params) |
| **Offline Mode** | ✅ Yes | ❌ No (needs backend) |
| **Distributed Scenes** | ✅ Yes (multiple servers) | ❌ No (single backend) |
| **Update Frequency** | Entity transforms | Shader uniforms (60Hz+) |
| **Use Case** | Game applications | Visual effects, always-online |

## Migration Guide

### From Entity Scene to Shader Scene

If you need to convert an entity-based scene to shader-driven:

```xml
<!-- Entity Scene (java-entity-scene.elyrion.xml) -->
<entity id="crate" asset="metal-crate">
  <pos x="0" y="0" z="0" />
</entity>

<!-- Becomes Shader Scene (coregfx-shader-scene.xml) -->
<instance id="metal_crate_01" template="pbr_standard">
  <geometry>
    <mesh src="models/crate.gltf" />
  </geometry>
  <transform>
    <position x="0" y="0" z="0" />
  </transform>
</instance>
```

### From Shader Scene to Entity Scene

If you need entity abstraction from shader-driven:

```xml
<!-- Shader Scene (coregfx-shader-scene.xml) -->
<instance id="smoke_01" template="volumetric_smoke">
  <uniforms>
    <uniform name="density" value="0.7" />
  </uniforms>
</instance>

<!-- Becomes Entity Scene (java-entity-scene.elyrion.xml) -->
<entity id="smoke" type="particle-system">
  <component type="VolumetricSmoke">
    <property name="density" value="0.7" />
  </component>
</entity>
```

## Examples in Project

### Entity Scene Example
```bash
# Used by Java client
java -jar game-client.jar --scene java-entity-scene.elyrion.xml
```

### Shader Scene Example
```bash
# Used by C++ renderer (direct shader control)
./elyrion.coregfx.renderer.exe coregfx-shader-scene.xml --shader-driven
```

## Recommended Format: Entity Scene

**For your Java + C++ architecture, use `java-entity-scene.elyrion.xml`:**

✅ Entities in Java (easy to develop/debug)
✅ C++ only renders (performance)
✅ Clean JNI interface (simple data transfer)
✅ Distributed scene loading (multiple servers)
✅ Offline capability (static scenes)

---

**Document Version**: 1.0
**Last Updated**: 2025-10-03