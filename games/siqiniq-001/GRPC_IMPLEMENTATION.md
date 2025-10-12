# Siqiniq-001 gRPC Implementation Guide

This document explains the gRPC service implementation for Siqiniq-001.

## Overview

Siqiniq-001 implements **two gRPC services** from arctic-formats:

1. **RenderStreamService** (Port 9000) - Core rendering stream
2. **ShaderGraphStreamingService** (Port 9000) - Advanced shader pipelines

**Note:** Both services currently run on port 9000 as Quarkus gRPC doesn't natively support multiple ports. In production, use a reverse proxy (Nginx/Envoy) to route to separate instances if needed.

## Implementation Status

### ✅ Completed

- [x] Service implementation classes created
  - `RenderStreamServiceImpl.java`
  - `ShaderGraphStreamingServiceImpl.java`
- [x] Supporting components
  - `RenderStreamManager.java` - Connection management
  - `ShaderGraphManager.java` - Shader graph management
  - `AdaptiveLODSystem.java` - Performance-based quality adaptation
- [x] Build configuration
  - pom.xml updated with gRPC plugin
  - Proto generation from arctic-formats configured
- [x] Infrastructure
  - Docker Compose exposes port 9000
  - Kubernetes manifests expose gRPC port
- [x] Configuration
  - application.properties with gRPC settings
  - Shader LOD system configuration

### 🔄 Pending (Requires arctic-formats integration)

- [ ] Uncomment proto imports once arctic-formats is linked
- [ ] Implement actual proto message handling
- [ ] Test bidirectional streaming
- [ ] Implement shader compilation pipeline

## Project Structure

```
siqiniq-001/
├── src/main/java/io/qimmiit/siqiniq/
│   ├── server/
│   │   ├── render/
│   │   │   ├── RenderStreamServiceImpl.java      # gRPC service implementation
│   │   │   └── RenderStreamManager.java          # Connection/broadcast management
│   │   └── shader/
│   │       ├── ShaderGraphStreamingServiceImpl.java  # Shader service implementation
│   │       ├── ShaderGraphManager.java           # Graph/node management
│   │       └── AdaptiveLODSystem.java            # Auto LOD adaptation
│   └── cms/
│       └── api/
│           └── AdminMetricsResource.java         # Admin REST API
├── src/main/proto/
│   └── arctic_network_integration.proto          # Integration proto
├── pom.xml                                        # Maven config with gRPC
└── src/main/resources/
    └── application.properties                     # gRPC + LOD config
```

## Service Implementation Details

### RenderStreamService (Port 9000)

**Purpose:** Real-time bidirectional rendering updates

**RPCs:**
- `StreamRenderingUpdates(stream ClientEvent) returns (stream RenderEvent)` - Bidirectional streaming
- `GetSceneSnapshot(SceneSnapshotRequest) returns (SceneSnapshot)` - Unary RPC

**Client → Server Events:**
- `ClientInputEvent` - Mouse, keyboard, gamepad
- `ClientCameraControl` - Camera movement
- `ClientEntityCommand` - Entity manipulation
- `ClientSceneCommand` - Scene commands

**Server → Client Events:**
- `SceneLoaded` - Scene initialization
- `EntitySpawned/Updated/Destroyed` - Entity changes
- `CameraUpdate` - Camera changes
- `LightUpdate` - Light changes
- `MaterialUpdate` - Material changes
- `FrameStats` - Performance metrics

**Implementation:**
```java
@GrpcService
public class RenderStreamServiceImpl {
    @Inject RenderStreamManager renderStreamManager;

    public StreamObserver<ClientEvent> streamRenderingUpdates(
            StreamObserver<RenderEvent> responseObserver) {
        // Connection management
        // Event processing
        // Broadcasting
    }
}
```

**Key Features:**
- Connection tracking per client
- Broadcast to all connected clients
- Per-connection event routing
- Metrics: active connections, events sent/received

### ShaderGraphStreamingService (Port 9000)

**Purpose:** Advanced shader pipeline management with adaptive LOD

**RPCs:**
- `StreamShaderGraphs(stream ShaderGraphCommand) returns (stream ShaderGraphResponse)` - Bidirectional
- `LoadGraph(LoadShaderGraphCmd) returns (ShaderGraphResponse)` - Unary
- `MonitorPerformance(RpcVoid) returns (stream PerformanceReport)` - Server streaming

**Commands (Client → Server):**
- `LoadShaderGraphCmd` - Load complete graph
- `UpdateNodeCmd` - Hot-reload single node
- `SetPerformanceBudgetCmd` - Set performance budget
- `SetLODLevelCmd` - Manual LOD level
- `ReplaceNodeCmd` - Swap nodes
- `RecompileGraphCmd` - Recompile graph
- `SwapShaderCmd` - Swap entire shader

**Responses (Server → Client):**
- `GraphLoadedResponse` - Graph loaded with cost
- `NodeUpdatedResponse` - Node updated status
- `LODChangedResponse` - LOD change details
- `PerformanceReport` - Budget utilization
- `ShaderCompilationError` - Compilation errors

**Adaptive LOD System:**

| LOD | Quality | Features | Cost |
|-----|---------|----------|------|
| 0 | Full PBR | IBL + Normal + Specular | ~8-10ms |
| 1 | PBR -IBL | Normal + Specular | ~5-6ms |
| 2 | Simplified | Specular only | ~3-4ms |
| 3 | Basic | Lambert diffuse | ~1-2ms |

**Automatic Adaptation:**
- Frame time > 16.67ms → Drop LOD
- Frame time < 12ms (70% budget) → Increase LOD
- 2-second cooldown between changes

**Implementation:**
```java
@ApplicationScoped
public class AdaptiveLODSystem {
    @Scheduled(every = "100ms")
    void monitorPerformance() {
        float avgFrameTime = calculateAverageFrameTime();
        if (shouldChangeLOD(avgFrameTime)) {
            adaptLOD(avgFrameTime);
        }
        sendPerformanceReports(avgFrameTime);
    }
}
```

## Configuration

### application.properties

```properties
# gRPC Configuration
quarkus.grpc.server.port=9000
quarkus.grpc.server.host=0.0.0.0
quarkus.grpc.server.max-inbound-message-size=4194304
quarkus.grpc.server.enable-reflection-service=true

# Shader Graph Configuration
siqiniq.shader.lod.auto-adapt=true
siqiniq.shader.lod.target-frame-time-ms=16.67
siqiniq.shader.lod.initial-level=0
siqiniq.shader.lod.change-cooldown-ms=2000
```

### Docker Compose

```yaml
siqiniq-server:
  ports:
    - "8080:8080"  # HTTP/REST/WebSocket
    - "9000:9000"  # gRPC (both services)
```

### Kubernetes

```yaml
ports:
- containerPort: 8080
  name: http
- containerPort: 9000
  name: grpc
```

## Building and Running

### 1. Link arctic-formats

```bash
# Ensure arctic-formats is available at ../../../arctic-formats/proto
# Or update pom.xml with correct path
```

### 2. Generate Proto Classes

```bash
# Maven will automatically generate Java classes from proto files
./mvnw clean compile
```

### 3. Build Application

**JVM mode:**
```bash
./mvnw clean package
java -jar target/quarkus-app/quarkus-run.jar
```

**Native mode:**
```bash
./mvnw clean package -Pnative
./target/siqiniq-001-1.0.0-SNAPSHOT-runner
```

### 4. Deploy with Docker

```bash
cd docker
docker-compose up -d
```

### 5. Deploy to Kubernetes

```bash
kubectl apply -f docker/k8s/
```

## Testing gRPC Services

### Using grpcurl

**List services:**
```bash
grpcurl -plaintext localhost:9000 list
```

**Describe service:**
```bash
grpcurl -plaintext localhost:9000 describe RenderStreamService
grpcurl -plaintext localhost:9000 describe ShaderGraphStreamingService
```

**Call unary RPC:**
```bash
grpcurl -plaintext -d '{"scene_id": "default"}' \
  localhost:9000 RenderStreamService/GetSceneSnapshot
```

**Call streaming RPC:**
```bash
grpcurl -plaintext -d @ \
  localhost:9000 RenderStreamService/StreamRenderingUpdates <<EOF
{"camera_control": {"movement": {"x": 1.0}}}
EOF
```

### Using BloomRPC/Postman

1. Load proto files from arctic-formats
2. Connect to `localhost:9000`
3. Test RPCs interactively

### C++ Client Example

```cpp
#include <grpcpp/grpcpp.h>
#include "arctic_network.grpc.pb.h"

// Connect to render stream
auto channel = grpc::CreateChannel("localhost:9000",
    grpc::InsecureChannelCredentials());
auto stub = RenderStreamService::NewStub(channel);

// Create bidirectional stream
grpc::ClientContext context;
auto stream = stub->StreamRenderingUpdates(&context);

// Send client events
ClientEvent event;
event.mutable_camera_control()->mutable_movement()->set_x(1.0f);
stream->Write(event);

// Receive render events
RenderEvent render_event;
while (stream->Read(&render_event)) {
    if (render_event.has_entity_updated()) {
        // Apply transform update
    }
}
```

## Monitoring and Metrics

### Admin REST API

**Metrics summary:**
```bash
curl http://localhost:8080/api/admin/metrics/summary
```

**Active connections:**
```bash
curl http://localhost:8080/api/admin/metrics/connections
```

**Performance metrics:**
```bash
curl http://localhost:8080/api/admin/metrics/performance
```

**Frame stats:**
```bash
curl http://localhost:8080/api/admin/metrics/frame-stats
```

### Prometheus Metrics

```
# RenderStreamService
siqiniq_render_stream_events_sent_total
siqiniq_render_stream_events_received_total
siqiniq_render_stream_latency_seconds
siqiniq_render_stream_active_connections

# ShaderGraphStreamingService
siqiniq_shader_graphs_loaded_total
siqiniq_shader_nodes_updated_total
siqiniq_shader_lod_changes_total
siqiniq_shader_compilation_duration_seconds
siqiniq_shader_cost_ms{node_id="...", lod_level="..."}
```

## Next Steps

### 1. Integrate arctic-formats

- Ensure arctic-formats submodule is initialized
- Verify proto files are accessible
- Uncomment proto imports in service implementations

### 2. Implement Message Handlers

- Complete event processing in RenderStreamServiceImpl
- Implement shader compilation in ShaderGraphManager
- Test bidirectional streaming

### 3. Add Unit Tests

```java
@QuarkusTest
public class RenderStreamServiceTest {
    @Test
    void testStreamRenderingUpdates() {
        // Test bidirectional streaming
    }
}
```

### 4. Performance Testing

- Load testing with multiple clients
- Measure frame time with different LOD levels
- Verify adaptive LOD behavior

### 5. Production Deployment

- Configure TLS for gRPC
- Set up reverse proxy for separate ports
- Configure resource limits
- Enable distributed tracing

## Troubleshooting

### Proto generation fails

**Issue:** Cannot find arctic-formats proto files

**Solution:**
```bash
# Check path in pom.xml
<additionalProtoPathElement>${project.basedir}/../../../arctic-formats/proto</additionalProtoPathElement>

# Verify arctic-formats exists
ls ../../../arctic-formats/proto
```

### gRPC service not starting

**Issue:** Port 9000 already in use

**Solution:**
```bash
# Check what's using port 9000
netstat -ano | findstr 9000

# Change port in application.properties
quarkus.grpc.server.port=9001
```

### No clients connecting

**Issue:** Firewall blocking port

**Solution:**
```bash
# Windows: Allow port through firewall
netsh advfirewall firewall add rule name="gRPC" dir=in action=allow protocol=TCP localport=9000

# Docker: Ensure port is exposed
docker ps | grep 9000
```

## References

- [SERVICES.md](SERVICES.md) - Detailed service documentation
- [ARCHITECTURE.md](ARCHITECTURE.md) - Overall architecture
- [INFRASTRUCTURE.md](INFRASTRUCTURE.md) - Infrastructure setup
- [Quarkus gRPC Guide](https://quarkus.io/guides/grpc-getting-started)
- [arctic-formats repository](../../../arctic-formats/)
