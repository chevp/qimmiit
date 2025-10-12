# Siqiniq-001 gRPC Services

Siqiniq-001 implementiert **zwei separate gRPC Services** aus arctic-formats:

## 1. RenderStreamService (Core)

**From:** `arctic_network.proto`
**Port:** `9000` (default gRPC)
**Purpose:** Real-time bidirectional rendering updates

### Service Definition

```protobuf
service RenderStreamService {
  // Bidirectional streaming: Client events ↔ Render events
  rpc StreamRenderingUpdates(stream ClientEvent) returns (stream RenderEvent);

  // Get scene snapshot (non-streaming)
  rpc GetSceneSnapshot(SceneSnapshotRequest) returns (SceneSnapshot);
}
```

### Client → Server Events

**ClientEvent types:**
- `ClientInputEvent` - Mouse, keyboard, gamepad input
- `ClientCameraControl` - Camera movement, rotation, zoom
- `ClientEntityCommand` - Entity selection, move, rotate, scale, delete
- `ClientSceneCommand` - Load/unload scenes, spawn entities

### Server → Client Events

**RenderEvent types:**
- `SceneLoaded` - Scene initialization with resource handles
- `EntitySpawned` - New entity in scene
- `EntityUpdated` - Transform/visibility changes
- `EntityDestroyed` - Entity removed
- `CameraUpdate` - Camera position/rotation/FOV changes
- `LightUpdate` - Light spawned/modified/destroyed
- `MaterialUpdate` - Runtime material parameter changes
- `FrameStats` - Performance metrics (FPS, draw calls, etc.)

### Example Flow

```
Client connects
    │
    ├─► Send: ClientEvent (Input/Camera/Entity commands)
    │
    └─► Receive: RenderEvent stream
            ├─► SceneLoaded (initial state)
            ├─► EntitySpawned (entities in scene)
            ├─► CameraUpdate (camera changes)
            └─► FrameStats (every frame)
```

### Use Cases

✅ **Real-time scene streaming** to Qimmiit renderer
✅ **Interactive 3D editing** (move/rotate entities)
✅ **Multiplayer scene synchronization**
✅ **Remote rendering** (server renders, streams to client)

---

## 2. ShaderGraphStreamingService (Advanced)

**From:** `arctic_shader_streaming.proto`
**Port:** `9001` (separate port)
**Purpose:** Advanced shader pipeline management with adaptive LOD

### Service Definition

```protobuf
service ShaderGraphStreamingService {
  // Bidirectional streaming for real-time shader graph updates
  rpc StreamShaderGraphs(stream ShaderGraphCommand) returns (stream ShaderGraphResponse);

  // Unary RPC for one-time graph loads
  rpc LoadGraph(LoadShaderGraphCmd) returns (ShaderGraphResponse);

  // Server streaming for performance telemetry
  rpc MonitorPerformance(arctic.common.RpcVoid) returns (stream PerformanceReport);
}
```

### Commands (Client → Server)

**ShaderGraphCommand types:**
- `LoadShaderGraphCmd` - Load complete shader graph
- `UpdateNodeCmd` - Hot-reload single shader node
- `SetPerformanceBudgetCmd` - Set performance budget (triggers adaptive LOD)
- `SetLODLevelCmd` - Manually set LOD level (0-3)
- `ReplaceNodeCmd` - Swap expensive node with cheaper alternative
- `RecompileGraphCmd` - Recompile entire graph after changes
- `SwapShaderCmd` - Swap entire shader (e.g., PBR → Unlit)

### Responses (Server → Client)

**ShaderGraphResponse types:**
- `GraphLoadedResponse` - Graph loaded with cost estimate
- `NodeUpdatedResponse` - Node updated, recompilation status
- `LODChangedResponse` - LOD changed, nodes disabled/enabled
- `PerformanceReport` - Budget utilization, recommendations
- `ShaderCompilationError` - Compilation error with line/column

### Adaptive LOD System

**Performance-based quality adjustment:**

| LOD Level | Quality | Features | Cost |
|-----------|---------|----------|------|
| 0 (Highest) | Full PBR | IBL, Normal mapping, Specular | ~8-10ms |
| 1 (High) | PBR - IBL | Normal mapping, Specular | ~5-6ms |
| 2 (Medium) | Simplified | Specular only | ~3-4ms |
| 3 (Low) | Basic | Lambert diffuse | ~1-2ms |

**Automatic adaptation:**
```
Frame time > 16.67ms → Drop LOD
Frame time < 12ms → Increase LOD
```

### Shader Node Library

**Pre-built PBR nodes from `pbr_khr.frag`:**

**High-cost nodes (>2ms):**
- IBL Contribution
- Microfacet Distribution
- Geometric Occlusion
- Specular Reflection
- Normal Mapping

**Medium-cost nodes (0.1-0.5ms):**
- sRGB to Linear conversion
- Tonemapping
- Metallic workflow conversion

**Low-cost nodes (<0.1ms):**
- Diffuse (Lambertian)
- Base color sampling
- Material parameters

### Example: Hot Reload Shader Node

**Client sends:**
```protobuf
UpdateNodeCmd {
  graph_id: "pbr_dynamic"
  node_id: "specular_reflection"
  updated_node: {
    glsl_source: "vec3 specularReflection(...) { /* new code */ }"
    spirv_binary: <compiled_spirv>
  }
  relink_connections: true
}
```

**Server responds:**
```protobuf
NodeUpdatedResponse {
  node_id: "specular_reflection"
  recompilation_required: true
  new_cost_ms: 1.8
}
```

**Result:** Shader recompiled on-the-fly, pipeline swapped without restart!

### Example: Adaptive Performance

**Scenario:** Frame time spikes to 20ms (below 60 FPS)

**Server automatically:**
1. Detects budget exceeded
2. Drops to LOD 2
3. Disables: IBL, Normal Mapping
4. Enables: Simple ambient, Vertex normals
5. New cost: 3.2ms (from 8.5ms)

**Client receives:**
```protobuf
LODChangedResponse {
  old_lod: 0
  new_lod: 2
  disabled_nodes: ["ibl_contribution", "normal_mapping"]
  enabled_nodes: ["simple_ambient", "vertex_normal"]
  new_total_cost_ms: 3.2
}
```

### Use Cases

✅ **Live shader editing** (hot reload GLSL code)
✅ **Performance adaptation** (automatic LOD based on FPS)
✅ **Node-based shader composition** (swap PBR modules)
✅ **Quality presets** (Low/Medium/High/Ultra)
✅ **GPU profiling** (cost per shader node)
✅ **Shader optimization** (identify expensive nodes)

---

## Service Separation

### Why Two Separate Services?

**RenderStreamService (Core):**
- 🎯 **Purpose:** Scene/entity streaming
- ⚡ **Frequency:** Every frame (60 FPS)
- 📦 **Data:** Transform updates, input events
- 🔄 **Usage:** Always active during rendering

**ShaderGraphStreamingService (Advanced):**
- 🎯 **Purpose:** Shader pipeline management
- ⚡ **Frequency:** On-demand (editor changes, LOD switches)
- 📦 **Data:** Shader code, compilation results
- 🔄 **Usage:** Development, dynamic quality adjustment

**Benefits of separation:**
- ✅ Independent scaling
- ✅ Different performance profiles
- ✅ Optional shader service (not needed for basic rendering)
- ✅ Cleaner API boundaries
- ✅ Separate monitoring/metrics

---

## Configuration

**application.properties:**
```properties
# RenderStreamService (Core)
quarkus.grpc.server.port=9000
quarkus.grpc.server.max-inbound-message-size=4194304

# ShaderGraphStreamingService (Advanced)
siqiniq.shader.grpc.port=9001
siqiniq.shader.grpc.enabled=true
siqiniq.shader.lod.auto-adapt=true
siqiniq.shader.lod.target-frame-time-ms=16.67
```

---

## Endpoints Summary

| Service | Protocol | Port | Endpoint | Purpose |
|---------|----------|------|----------|---------|
| RenderStream | gRPC | 9000 | `/RenderStreamService/StreamRenderingUpdates` | Scene streaming |
| RenderStream | gRPC | 9000 | `/RenderStreamService/GetSceneSnapshot` | Scene snapshot |
| ShaderGraph | gRPC | 9001 | `/ShaderGraphStreamingService/StreamShaderGraphs` | Shader updates |
| ShaderGraph | gRPC | 9001 | `/ShaderGraphStreamingService/LoadGraph` | Load graph |
| ShaderGraph | gRPC | 9001 | `/ShaderGraphStreamingService/MonitorPerformance` | Performance |
| Admin Metrics | REST | 8080 | `/api/admin/metrics/*` | Monitoring |
| Health | REST | 8080 | `/q/health` | Health check |
| Metrics | REST | 8080 | `/q/metrics` | Prometheus |

---

## Client Integration

### C++ Client (Qimmiit)

**RenderStreamService:**
```cpp
// Connect to render stream
auto channel = grpc::CreateChannel("siqiniq-server:9000", ...);
auto stub = RenderStreamService::NewStub(channel);

auto stream = stub->StreamRenderingUpdates(&context);

// Send client events
ClientEvent event;
event.mutable_camera_control()->mutable_movement()->set_x(delta.x);
stream->Write(event);

// Receive render events
RenderEvent render_event;
while (stream->Read(&render_event)) {
    if (render_event.has_entity_updated()) {
        // Apply transform update to renderer
    }
}
```

**ShaderGraphStreamingService:**
```cpp
// Connect to shader service
auto shader_channel = grpc::CreateChannel("siqiniq-server:9001", ...);
auto shader_stub = ShaderGraphStreamingService::NewStub(shader_channel);

// Load shader graph
LoadShaderGraphCmd cmd;
cmd.mutable_graph()->set_graph_id("pbr_dynamic");
ShaderGraphResponse response;
shader_stub->LoadGraph(&context, cmd, &response);

// Monitor performance
auto perf_stream = shader_stub->MonitorPerformance(&context, RpcVoid{});
PerformanceReport report;
while (perf_stream->Read(&report)) {
    if (report.budget_exceeded()) {
        // LOD will auto-adapt
    }
}
```

---

## Performance Metrics

### RenderStreamService Metrics

```
siqiniq_render_stream_events_sent_total
siqiniq_render_stream_events_received_total
siqiniq_render_stream_latency_seconds
siqiniq_render_stream_active_connections
```

### ShaderGraphStreamingService Metrics

```
siqiniq_shader_graphs_loaded_total
siqiniq_shader_nodes_updated_total
siqiniq_shader_lod_changes_total
siqiniq_shader_compilation_duration_seconds
siqiniq_shader_cost_ms{node_id="...", lod_level="..."}
```

---

## Next Steps

1. **Implement RenderStreamService** (Priority 1)
   - Core functionality for scene streaming
   - Required for basic rendering

2. **Implement ShaderGraphStreamingService** (Priority 2)
   - Advanced shader management
   - Optional but powerful for quality/performance

3. **Add arctic-formats dependency**
   - Link proto definitions
   - Generate Java classes

4. **Testing**
   - Unit tests for service implementations
   - Integration tests with Qimmiit client
   - Performance benchmarks

See `ARCHITECTURE.md` for overall system design.
