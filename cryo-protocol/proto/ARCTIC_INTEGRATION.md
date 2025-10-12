# Arctic-Formats Integration into Cryo-Protocol

Basierend auf der Analyse von `arctic-formats/proto` wurden folgende fehlende Konzepte zu `cryo-protocol/proto` hinzugefügt.

## Hinzugefügte Dateien

### 1. **[cryo_container.proto](cryo_container.proto)** - Unified gRPC Container System ⭐

**Zweck**: Einheitlicher Container für ALLE gRPC-Kommunikation

**Hauptkonzepte**:
```protobuf
message CryoContainer {
  google.protobuf.Any payload = 1;  // Kann JEDE Message enthalten
  string message_id = 100;
  Timestamp timestamp = 101;
  string source = 102;
  string destination = 103;
  MessagePriority priority = 104;
  map<string, string> metadata = 105;
}
```

**Service Definition**:
```protobuf
service CryoContainerService {
  rpc Send(CryoRequest) returns (CryoResponse);           // Unary
  rpc SendBatch(CryoBatch) returns (CryoResponse);        // Batch
  rpc Subscribe(CryoSubscription) returns (stream CryoStream);  // Server streaming
  rpc Upload(stream CryoStream) returns (CryoResponse);   // Client streaming
  rpc Connect(stream CryoContainer) returns (stream CryoContainer);  // Bidirectional
  rpc Publish(CryoContainer) returns (CryoAck);           // Pub/Sub
}
```

**Vorteile**:
- ✅ Single service endpoint für alle Operationen
- ✅ Unified metadata (logging, auth, metrics)
- ✅ Forward/backward compatible
- ✅ `google.protobuf.Any` statt giant oneof
- ✅ Routing und Middleware-Support

**Vergleich mit Arctic**:
| Arctic | Cryo | Status |
|--------|------|--------|
| ArcticContainer | CryoContainer | ✅ Portiert |
| ArcticBatch | CryoBatch | ✅ Portiert + BatchMode |
| ArcticStream | CryoStream | ✅ Portiert + StreamFlags |
| - | CryoSubscription | ⭐ Erweitert (Pub/Sub) |
| - | CryoAck | ⭐ Neu (Reliability) |

---

### 2. **[cryo_project.proto](asset_management/cryo_project.proto)** - Project/Manifest System ⭐

**Zweck**: Asset-Organisation und Dependency-Management

**Hauptkonzepte**:
```protobuf
message CryoProject {
  string project_id = 1;
  ProjectSettings settings = 10;
  AssetRegistry asset_registry = 20;
  repeated SceneManifest scenes = 30;
  repeated PrefabManifest prefabs = 40;
  repeated MaterialManifest materials = 50;
  repeated ShaderGraphManifest shader_graphs = 60;
}
```

**Asset Registry mit Dependency-Graph**:
```protobuf
message AssetRegistry {
  repeated AssetEntry assets = 1;
  repeated AssetDependency dependencies = 2;
  AssetIndex index = 3;  // Fast lookups
}

message AssetDependency {
  string asset_id = 1;
  repeated string depends_on = 2;
  DependencyType type = 3;  // HARD, SOFT, WEAK
}
```

**Storage Backends**:
- FILE_SYSTEM (local files)
- SQLITE (database)
- REMOTE_HTTP (REST API)
- S3 (AWS S3 / compatible)
- GIT_LFS (Git Large File Storage)

**Asset Types**:
- SCENE, PREFAB, MATERIAL, SHADER_GRAPH
- MESH, TEXTURE, AUDIO, ANIMATION
- PARTICLE_SYSTEM, UI_LAYOUT, SCRIPT

**Load Modes**:
- LAZY (on-demand)
- EAGER (preload all)
- STREAMING (during rendering)
- PROGRESSIVE (LOD-based)

**Vergleich mit Arctic**:
| Arctic | Cryo | Status |
|--------|------|--------|
| ArcticProject | CryoProject | ✅ Portiert |
| StorageBackend (3 types) | StorageBackend (5 types) | ⭐ Erweitert (S3, Git LFS) |
| AssetRegistry | AssetRegistry + Index | ⭐ Erweitert (Fast lookups) |
| LoadMode (3) | LoadMode (4) | ⭐ Erweitert (PROGRESSIVE) |

---

### 3. **[cryo_runtime.proto](asset_management/cryo_runtime.proto)** - Runtime Optimized Format ⭐

**Zweck**: Optimiertes Format für den Renderer (compiled, cached)

**Hauptkonzepte**:
```protobuf
message RuntimeScene {
  repeated RuntimeEntity entities = 10;
  CompiledShaderCache shader_cache = 11;    // SPIR-V bytecode
  MaterialCache material_cache = 12;
  MeshCache mesh_cache = 13;
  TextureCache texture_cache = 14;
}
```

**Compiled Shader Cache**:
```protobuf
message CompiledShaderProgram {
  bytes vertex_spirv = 10;
  bytes fragment_spirv = 11;
  bytes geometry_spirv = 12;
  bytes compute_spirv = 15;
  ShaderReflection reflection = 20;  // Uniforms, inputs, outputs
}
```

**Material Cache (GPU-ready)**:
```protobuf
message CompiledMaterial {
  uint32 shader_program_handle = 10;
  map<string, CryoUniformValue> uniforms = 20;
  repeated uint32 texture_handles = 30;
  RenderState render_state = 40;
}
```

**Mesh Cache (optimized vertex data)**:
```protobuf
message CompiledMesh {
  bytes vertex_buffer = 10;
  bytes index_buffer = 11;
  VertexLayout layout = 20;
  repeated MeshLOD lods = 50;         // Multiple LOD levels
  repeated Submesh submeshes = 60;
}
```

**Runtime Entities (handle-based)**:
```protobuf
message RuntimeEntity {
  uint32 material_handle = 10;  // Index into cache (not full data!)
  uint32 mesh_handle = 11;
  LODConfiguration lod = 40;
}
```

**Vergleich mit Arctic**:
| Arctic | Cryo | Status |
|--------|------|--------|
| RuntimeScene | RuntimeScene | ✅ Portiert |
| CompiledShaderProgram | CompiledShaderProgram | ✅ Portiert + compute_spirv |
| MaterialCache | MaterialCache | ✅ Portiert |
| MeshCache | MeshCache | ✅ Portiert + Submeshes |
| TextureCache (8 formats) | TextureCache (11 formats) | ⭐ Erweitert (ASTC, ETC2) |

---

### 4. **[cryo_authoring.proto](asset_management/cryo_authoring.proto)** - Authoring Editor Format ⭐

**Zweck**: High-Level Editor-Format (uncompiled, symbolic references)

**Hauptkonzepte**:
```protobuf
message AuthoredScene {
  repeated AuthoredEntity entities = 10;
  repeated ShaderGraphDefinition shader_graphs = 11;  // NOT compiled!
  repeated MaterialDefinition materials = 12;
  EnvironmentSettings environment = 21;
}
```

**Shader Graph Definition (visual node-based)**:
```protobuf
message ShaderGraphDefinition {
  repeated ShaderNode nodes = 10;
  repeated NodeConnection connections = 11;
}

message ShaderNode {
  string node_type = 2;  // "TextureSample", "Add", "Multiply", etc.
  float ui_x = 10;       // Editor position
  float ui_y = 11;
  map<string, NodeParameter> parameters = 20;
}
```

**Material Definitions (multiple types)**:
```protobuf
message MaterialDefinition {
  oneof material_type {
    ShaderGraphReference shader_graph = 10;
    PBRMaterialParams pbr_params = 11;
    UnlitMaterialParams unlit_params = 12;
    ToonMaterialParams toon_params = 13;  // ⭐ Cryo addition
  }
}
```

**Procedural Mesh Generation**:
```protobuf
message ProceduralMeshDefinition {
  enum MeshType {
    CUBE, SPHERE, PLANE, CYLINDER, CONE, TORUS, CAPSULE
  }
  oneof type_params {
    SphereParams sphere = 20;
    CylinderParams cylinder = 21;
    TorusParams torus = 22;
  }
}
```

**Scene Compilation (Authoring → Runtime)**:
```protobuf
message CompileSceneRequest {
  AuthoredScene authoring_scene = 1;
  CompilationOptions options = 2;
}

message CompilationOptions {
  string target_platform = 1;  // "vulkan", "d3d12", "metal"
  bool optimize_meshes = 2;
  bool compress_textures = 3;
  bool generate_lods = 4;
  bool bake_lighting = 6;
}
```

**Vergleich mit Arctic**:
| Arctic | Cryo | Status |
|--------|------|--------|
| AuthoredScene | AuthoredScene | ✅ Portiert |
| ShaderGraphDefinition | ShaderGraphDefinition | ✅ Portiert |
| MaterialDefinition | MaterialDefinition | ⭐ Erweitert (ToonMaterialParams) |
| ProceduralMeshDefinition (5 types) | ProceduralMeshDefinition (7 types) | ⭐ Erweitert (TORUS, CAPSULE) |
| - | CompileSceneRequest | ⭐ Neu (Compilation API) |
| - | PrefabDefinition | ⭐ Neu (Prefab system) |

---

### 5. **[cryo_network.proto](sync/cryo_network.proto)** - Network Streaming Protocol ⭐

**Zweck**: Real-time bidirectional streaming für Renderer-Updates

**Hauptkonzepte**:
```protobuf
service RenderStreamService {
  rpc StreamRenderingUpdates(stream ClientEvent) returns (stream RenderEvent);
  rpc GetSceneSnapshot(SceneSnapshotRequest) returns (SceneSnapshot);
  rpc SubscribeToEntity(SubscribeRequest) returns (stream RenderEvent);
}
```

**Render Events (Server → Client)**:
```protobuf
message RenderEvent {
  oneof event_type {
    SceneLoaded scene_loaded = 10;
    EntitySpawned entity_spawned = 11;
    EntityUpdated entity_updated = 12;    // ⭐ Delta-based!
    EntityDestroyed entity_destroyed = 13;
    CameraUpdate camera_update = 14;
    LightUpdate light_update = 15;
    MaterialUpdate material_update = 16;
    FrameStats frame_stats = 17;
  }
}
```

**Delta-basierte Entity-Updates**:
```protobuf
message EntityUpdated {
  string entity_id = 1;
  oneof update_type {
    CryoTransform transform_update = 10;
    VisibilityUpdate visibility_update = 11;
    MaterialSwap material_swap = 12;
    TransformDelta transform_delta = 14;  // ⭐ More efficient!
  }
}

message TransformDelta {
  CryoVec3 position_delta = 1;  // Only send change!
  CryoVec4 rotation_delta = 2;
  CryoVec3 scale_delta = 3;
}
```

**Client Events (Client → Server)**:
```protobuf
message ClientEvent {
  oneof event_type {
    ClientInputEvent input_event = 10;
    ClientCameraControl camera_control = 11;
    ClientEntityCommand entity_command = 12;
    ClientSceneCommand scene_command = 13;
  }
}
```

**Frame Statistics**:
```protobuf
message FrameStats {
  float frame_time_ms = 10;
  float fps = 11;
  uint32 draw_calls = 20;
  uint32 triangles_rendered = 22;
  float gpu_time_ms = 41;
  int64 gpu_memory_used_bytes = 50;
}
```

**Vergleich mit Arctic**:
| Arctic | Cryo | Status |
|--------|------|--------|
| RenderStreamService | RenderStreamService | ✅ Portiert |
| EntityUpdated | EntityUpdated | ⭐ Erweitert (TransformDelta) |
| ClientInputEvent | ClientInputEvent | ⭐ Erweitert (TouchEvent) |
| FrameStats (6 metrics) | FrameStats (9 metrics) | ⭐ Erweitert (Memory) |
| - | SubscribeToEntity | ⭐ Neu (Entity subscription) |
| - | NetworkStats | ⭐ Neu (Bandwidth/latency) |

---

## Konzeptvergleich: Arctic vs. Cryo

### Architektur-Übersicht

```
ARCTIC-FORMATS                      CRYO-PROTOCOL
┌──────────────────────┐           ┌──────────────────────┐
│ arctic_container     │ ────→     │ cryo_container       │
│ - ArcticContainer    │           │ - CryoContainer      │
│ - ArcticBatch        │           │ - CryoBatch          │
│ - ArcticStream       │           │ - CryoStream         │
│                      │           │ + CryoSubscription   │  ⭐
│                      │           │ + CryoAck            │  ⭐
└──────────────────────┘           └──────────────────────┘

┌──────────────────────┐           ┌──────────────────────┐
│ arctic_project       │ ────→     │ cryo_project         │
│ - AssetRegistry      │           │ - AssetRegistry      │
│ - SceneManifest      │           │ - SceneManifest      │
│ - 3 Storage backends │           │ - 5 Storage backends │  ⭐
└──────────────────────┘           └──────────────────────┘

┌──────────────────────┐           ┌──────────────────────┐
│ arctic_runtime       │ ────→     │ cryo_runtime         │
│ - RuntimeScene       │           │ - RuntimeScene       │
│ - CompiledShader     │           │ - CompiledShader     │
│ - MaterialCache      │           │ - MaterialCache      │
│ - MeshCache          │           │ - MeshCache          │
│ - TextureCache       │           │ - TextureCache       │
└──────────────────────┘           └──────────────────────┘

┌──────────────────────┐           ┌──────────────────────┐
│ arctic_authoring     │ ────→     │ cryo_authoring       │
│ - AuthoredScene      │           │ - AuthoredScene      │
│ - ShaderGraphDef     │           │ - ShaderGraphDef     │
│ - MaterialDef        │           │ - MaterialDef        │
│                      │           │ + PrefabDefinition   │  ⭐
│                      │           │ + CompileScene API   │  ⭐
└──────────────────────┘           └──────────────────────┘

┌──────────────────────┐           ┌──────────────────────┐
│ arctic_network       │ ────→     │ cryo_network         │
│ - RenderStreamSvc    │           │ - RenderStreamSvc    │
│ - RenderEvent        │           │ - RenderEvent        │
│ - ClientEvent        │           │ - ClientEvent        │
│                      │           │ + NetworkStats       │  ⭐
│                      │           │ + EntitySubscription │  ⭐
└──────────────────────┘           └──────────────────────┘
```

---

## Unterschiede und Erweiterungen

### 1. Container System

**Arctic**:
- Einfaches `oneof` für Message-Types
- Keine Pub/Sub-Unterstützung
- Keine ACK-Mechanismen

**Cryo** ⭐:
- `google.protobuf.Any` (flexibler)
- `CryoSubscription` (Pub/Sub mit Filtern)
- `CryoAck` (Reliable streaming)
- `correlation_id`, `session_id`, `user_id` (Tracing)

### 2. Project System

**Arctic**:
- 3 Storage backends (FILE_SYSTEM, SQLITE, REMOTE_HTTP)
- LoadMode: LAZY, EAGER, STREAMING

**Cryo** ⭐:
- 5 Storage backends (+ S3, GIT_LFS)
- LoadMode: LAZY, EAGER, STREAMING, **PROGRESSIVE** (LOD-based)
- `AssetIndex` für Fast Lookups
- `AssetPriority` (LOWEST to CRITICAL)
- `DependencyType` (HARD, SOFT, WEAK)
- `RenderingDefaults` (Project-wide settings)

### 3. Runtime Format

**Arctic**:
- 8 Texture-Formate
- Keine Submesh-Unterstützung
- Keine Stencil-Unterstützung

**Cryo** ⭐:
- 11 Texture-Formate (+ ASTC, ETC2, RGB32F)
- `Submesh` Support (Multi-material meshes)
- `RenderState` mit Stencil
- `IBLData` (Image-Based Lighting)
- `OptimizeRuntimeSceneRequest` (Optimization API)

### 4. Authoring Format

**Arctic**:
- 5 Procedural mesh types
- 2 Material types (PBR, Unlit)
- Keine Prefab-Unterstützung
- Keine Compilation API

**Cryo** ⭐:
- 7 Procedural mesh types (+ TORUS, CAPSULE)
- 3 Material types (PBR, Unlit, **Toon**)
- `PrefabDefinition` + Instancing
- `CompileSceneRequest` (Authoring → Runtime)
- `InstantiatePrefabRequest` (Runtime prefab spawning)

### 5. Network Protocol

**Arctic**:
- Basic streaming
- 6 Frame-Stats-Metriken
- Keine Entity-Subscription
- Kein Touch-Input

**Cryo** ⭐:
- **Entity-Subscription** (SubscribeToEntity)
- 9 Frame-Stats-Metriken (+ Memory usage)
- **NetworkStats** (Bandwidth, latency, packet loss)
- **TouchEvent** (Multi-touch support)
- **TransformDelta** (More efficient than full transform)

---

## Integration in bestehende Cryo-Struktur

Die neuen Dateien wurden wie folgt eingeordnet:

```
cryo-protocol/proto/
├── cryo_container.proto              ⭐ NEU (Root level - unified protocol)
│
├── asset_management/
│   ├── cryo_project.proto            ⭐ NEU
│   ├── cryo_runtime.proto            ⭐ NEU
│   ├── cryo_authoring.proto          ⭐ NEU
│   ├── cryo_asset_gltf.proto         (bereits vorhanden)
│   ├── cryo_asset_gltf_binary.proto  (bereits vorhanden)
│   ├── cryo_asset_gltf_commands.proto (bereits vorhanden)
│   ├── cryo_asset_texture.proto      (bereits vorhanden)
│   └── cryo_asset_mesh.proto         (bereits vorhanden)
│
└── sync/
    ├── cryo_network.proto            ⭐ NEU
    ├── cryo_scene_update.proto       (bereits vorhanden)
    ├── cryo_sync.proto               (bereits vorhanden)
    ├── batch_commands.proto          (bereits vorhanden)
    ├── scene_update_delta.proto      (bereits vorhanden)
    ├── scene_keyframes.proto         (bereits vorhanden)
    ├── scene_sequences.proto         (bereits vorhanden)
    └── scene_state_sync.proto        (bereits vorhanden)
```

---

## Anwendungsfälle

### 1. Unified Container für alle Operationen

**Vorher** (ohne Container):
```cpp
// Separate services für jede Operation
SceneService::LoadScene(scene_request);
MaterialService::UpdateMaterial(material_request);
EntityService::CreateEntity(entity_request);
```

**Nachher** (mit CryoContainer):
```cpp
// Single service für alles
CryoContainer container;
container.mutable_payload()->PackFrom(scene_request);
container.set_source("cryo-editor");
container.set_priority(PRIORITY_HIGH);
CryoContainerService::Send(container);
```

### 2. Project-based Asset Management

```protobuf
// Load project manifest
CryoProject project = LoadProject("MyGame");

// Query assets by type
FindAssetsRequest request;
request.mutable_by_type()->set_asset_type(MATERIAL);
FindAssetsResponse materials = FindAssets(request);

// Load dependencies automatically
for (auto& asset : materials.assets()) {
  LoadAssetWithDependencies(asset.asset_id);
}
```

### 3. Authoring → Runtime Compilation

```protobuf
// Editor saves AuthoredScene
AuthoredScene authored_scene;
authored_scene.add_entities(entity);
SaveAuthoredScene(authored_scene, "scene.authored.pb");

// Compile to RuntimeScene
CompileSceneRequest request;
*request.mutable_authoring_scene() = authored_scene;
request.mutable_options()->set_target_platform("vulkan");
request.mutable_options()->set_optimize_meshes(true);

CompileSceneResponse response = CompileScene(request);
RuntimeScene runtime_scene = response.runtime_scene();
```

### 4. Real-time Network Streaming

```cpp
// Server streams updates to client
auto stream = RenderStreamService::StreamRenderingUpdates(client_input);

while (stream->Read(&event)) {
  if (event.has_entity_updated()) {
    // Delta-based update (efficient!)
    auto delta = event.entity_updated().transform_delta();
    ApplyPositionDelta(delta.position_delta());
  }
}
```

---

## Zusammenfassung

| Feature | Arctic | Cryo | Vorteil |
|---------|--------|------|---------|
| **Container** | ArcticContainer | CryoContainer | ✅ Pub/Sub, ACK, Tracing |
| **Storage** | 3 backends | 5 backends | ⭐ S3, Git LFS |
| **Load Modes** | 3 modes | 4 modes | ⭐ Progressive LOD |
| **Textures** | 8 formats | 11 formats | ⭐ Mobile (ASTC, ETC2) |
| **Materials** | 2 types | 3 types | ⭐ Toon shading |
| **Meshes** | 5 types | 7 types | ⭐ Torus, Capsule |
| **Prefabs** | ❌ | ✅ | ⭐ Prefab system |
| **Compilation** | ❌ | ✅ | ⭐ Authoring → Runtime |
| **Network** | Basic | Extended | ⭐ NetworkStats, Touch |
| **Subscriptions** | ❌ | ✅ | ⭐ Entity subscriptions |

**Alle Arctic-Konzepte wurden erfolgreich portiert und erweitert!** 🚀

---

## Nächste Schritte

1. **CryoContainer Service implementieren** (C++/Rust)
   - gRPC Server mit unified endpoint
   - Payload routing basierend auf `type_url`

2. **Project Loading Pipeline**
   - Asset Registry aufbauen
   - Dependency Resolution
   - Multi-backend Support (FILE_SYSTEM, S3, etc.)

3. **Authoring → Runtime Compiler**
   - Shader Graph → SPIR-V Compilation
   - Mesh optimization (LOD generation)
   - Texture compression (BC7, ASTC, etc.)

4. **Network Streaming implementieren**
   - RenderStreamService Server
   - Delta-basierte Entity-Updates
   - Client-side interpolation

5. **Testing & Validation**
   - Unit tests für alle Services
   - Integration tests für end-to-end flows
   - Performance benchmarks
