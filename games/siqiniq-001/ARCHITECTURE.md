# Siqiniq-001 Architecture

## Overview

Siqiniq-001 is a **Render Stream Server** that implements the `RenderStreamService` from `arctic_network.proto`. It provides real-time bidirectional streaming for rendering updates to clients.

## Core Purpose

This server is **NOT** a traditional game server with business logic. Instead, it:

✅ **Implements `RenderStreamService`** (from arctic-formats)
- Bidirectional gRPC streaming
- Client events → Render events
- Real-time scene updates

✅ **Provides Admin/Metrics Endpoints**
- Connection monitoring
- Performance metrics
- Resource usage tracking
- Frame statistics

❌ **Does NOT provide** business logic APIs (worlds, items, characters)

## Architecture Diagram

```
┌─────────────┐         gRPC Stream          ┌─────────────┐
│             │◄──────────────────────────────│             │
│   Client    │   StreamRenderingUpdates()   │   Siqiniq   │
│  (Qimmiit)  │──────────────────────────────►│   Server    │
│             │    ClientEvent/RenderEvent    │             │
└─────────────┘                               └─────────────┘
                                                     │
                                                     │ REST/Metrics
                                                     ▼
                                              ┌─────────────┐
                                              │    Admin    │
                                              │   Console   │
                                              └─────────────┘
```

## Components

### 1. RenderStreamService Implementation (gRPC)

**Proto Definition** (from `arctic_network.proto`):
```protobuf
service RenderStreamService {
  rpc StreamRenderingUpdates(stream ClientEvent) returns (stream RenderEvent);
  rpc GetSceneSnapshot(SceneSnapshotRequest) returns (SceneSnapshot);
}
```

**Implementation Location:**
- Java Package: `io.qimmiit.siqiniq.server.render`
- Uses Quarkus gRPC

**Events Handled:**

**Client → Server (ClientEvent):**
- `ClientInputEvent` - Mouse, keyboard, gamepad input
- `ClientCameraControl` - Camera movement/rotation
- `ClientEntityCommand` - Entity manipulation
- `ClientSceneCommand` - Scene loading

**Server → Client (RenderEvent):**
- `SceneLoaded` - Scene initialization
- `EntitySpawned` - New entity
- `EntityUpdated` - Entity transform/visibility
- `EntityDestroyed` - Entity removal
- `CameraUpdate` - Camera changes
- `LightUpdate` - Lighting changes
- `MaterialUpdate` - Material parameters
- `FrameStats` - Performance metrics

### 2. Admin Metrics API (REST)

**Endpoints:**

```
GET /api/admin/metrics/summary          - Overall server health
GET /api/admin/metrics/connections      - Active connections detail
GET /api/admin/metrics/performance      - CPU, Memory, Frame stats
GET /api/admin/metrics/resources        - Scene/Entity counts
GET /api/admin/metrics/frame-stats      - Rendering statistics
POST /api/admin/metrics/reset           - Reset counters
```

**Purpose:**
- Monitor active users
- Track performance issues
- Resource usage
- Connection health

### 3. Health & Observability

**Built-in Quarkus Endpoints:**
```
GET /q/health           - Overall health
GET /q/health/live      - Liveness probe
GET /q/health/ready     - Readiness probe
GET /q/metrics          - Prometheus metrics
GET /swagger-ui         - API documentation
```

## Data Flow

### 1. Client Connection Flow

```
1. Client connects via gRPC
   └─► StreamRenderingUpdates()

2. Server sends initial SceneLoaded event
   └─► Includes materials, meshes, textures handles
   └─► Initial camera position
   └─► Environment state

3. Bidirectional stream established
   ├─► Client sends: Input/Camera/Entity commands
   └─► Server sends: Entity/Camera/Light updates + FrameStats
```

### 2. Render Event Stream

```
Server Game Loop (20 TPS):
┌─────────────────────────┐
│  1. Process Client      │
│     Input Events        │
├─────────────────────────┤
│  2. Update Game State   │
│     - Entities          │
│     - Camera            │
│     - Lighting          │
├─────────────────────────┤
│  3. Generate Render     │
│     Events              │
├─────────────────────────┤
│  4. Stream to Clients   │
│     - EntityUpdated     │
│     - CameraUpdate      │
│     - FrameStats        │
└─────────────────────────┘
```

## Technology Stack

### Core
- **Quarkus 3.17** - Framework
- **gRPC** - RenderStreamService implementation
- **Protocol Buffers** - arctic-formats integration
- **Java 21** - Language

### Infrastructure
- **MariaDB** - Connection metadata, session tracking
- **Redis** - Client session cache, frame data cache
- **WebSocket** - Fallback for clients without gRPC

### Observability
- **Micrometer** - Metrics
- **Prometheus** - Metrics collection
- **Grafana** - Visualization
- **OpenTelemetry** - Tracing

## Database Schema

**Purpose:** Track connections and sessions (NOT game data)

```sql
-- Client connections
CREATE TABLE connections (
    id BIGINT PRIMARY KEY,
    client_id VARCHAR(255),
    ip_address VARCHAR(45),
    connected_at TIMESTAMP,
    disconnected_at TIMESTAMP,
    frames_sent INT,
    events_received INT
);

-- Session cache (also in Redis)
CREATE TABLE sessions (
    session_id VARCHAR(255) PRIMARY KEY,
    client_id VARCHAR(255),
    scene_id VARCHAR(255),
    last_activity TIMESTAMP
);

-- Performance logs
CREATE TABLE performance_logs (
    id BIGINT PRIMARY KEY,
    timestamp TIMESTAMP,
    fps FLOAT,
    frame_time_ms FLOAT,
    active_connections INT,
    cpu_usage FLOAT,
    memory_mb INT
);
```

## Configuration

**application.properties:**
```properties
# gRPC
quarkus.grpc.server.port=9000
quarkus.grpc.server.enable-reflection=true

# Database - Connection tracking only
quarkus.datasource.jdbc.url=jdbc:mariadb://localhost:3306/siqiniq

# Redis - Session cache
quarkus.redis.hosts=redis://localhost:6379

# Game Server
siqiniq.game.tick-rate=20
siqiniq.game.max-connections=100
siqiniq.stream.buffer-size=1024

# Performance
siqiniq.metrics.frame-stats-interval=60
siqiniq.metrics.export-to-db=true
```

## Key Differences from Traditional Game Server

| Feature | Traditional Game Server | Siqiniq-001 |
|---------|------------------------|-------------|
| **Purpose** | Business logic + rendering | Rendering stream only |
| **Protocol** | Custom TCP/WebSocket | gRPC (arctic_network) |
| **Data** | Game state (items, inventory) | Render events only |
| **APIs** | REST for content CRUD | Admin metrics only |
| **Database** | Game data storage | Connection tracking |
| **Events** | Gameplay events | Render events |

## Performance Characteristics

**Target Metrics:**
- **20 TPS** - Game tick rate
- **60 FPS** - Target frame rate
- **100 concurrent connections** - Per instance
- **< 16ms** - Frame generation time
- **< 5ms** - Event processing latency

**Resource Usage:**
- **JVM**: ~1GB RAM, 2 CPU cores
- **Native**: ~256MB RAM, 1 CPU core
- **Network**: ~10KB/s per connection (compressed)

## Scaling Strategy

### Horizontal Scaling

```
         Load Balancer (gRPC)
                │
        ┌───────┴───────┐
        ▼               ▼
  Siqiniq-001     Siqiniq-001
   Instance 1      Instance 2
        │               │
        └───────┬───────┘
                ▼
         Shared Redis
    (Session coordination)
```

**Kubernetes HPA:**
- Min replicas: 2
- Max replicas: 10
- Scale on: CPU > 70%, Memory > 80%

### Client Distribution

- Sticky sessions (Redis-based)
- Session migration support
- Graceful shutdown (drain connections)

## Monitoring

### Key Metrics

**Connection Metrics:**
- `siqiniq_connections_active`
- `siqiniq_connections_total`
- `siqiniq_events_received_total`
- `siqiniq_events_sent_total`

**Performance Metrics:**
- `siqiniq_frame_time_seconds`
- `siqiniq_fps`
- `siqiniq_tick_duration_seconds`
- `siqiniq_stream_latency_seconds`

**Resource Metrics:**
- `jvm_memory_used_bytes`
- `jvm_gc_pause_seconds_sum`
- `system_cpu_usage`

### Admin Console Queries

**Active Users:**
```bash
curl http://localhost:8080/api/admin/metrics/connections
```

**Performance Issues:**
```bash
curl http://localhost:8080/api/admin/metrics/performance
# Look for: high frame_time_ms, low fps, high cpu_load
```

**Resource Usage:**
```bash
curl http://localhost:8080/api/admin/metrics/summary
# Check: memory_percent, connection_rate
```

## Integration with Qimmiit Client

The Qimmiit client (C++ with coregfx) connects to this server:

```cpp
// Client code (pseudo)
auto channel = grpc::CreateChannel("siqiniq-server:9000");
auto stub = RenderStreamService::NewStub(channel);

// Start bidirectional stream
auto stream = stub->StreamRenderingUpdates(&context);

// Send client events
ClientEvent event;
event.mutable_input_event()->/* ... */;
stream->Write(event);

// Receive render events
RenderEvent render_event;
while (stream->Read(&render_event)) {
    // Apply to renderer
}
```

## Future Enhancements

- [ ] Scene state persistence (Redis)
- [ ] Replay system (record RenderEvents)
- [ ] Multi-scene support
- [ ] Advanced client routing
- [ ] Compression for large scenes
- [ ] Delta compression for EntityUpdated
- [ ] Predictive client-side rendering

## See Also

- `arctic-formats/proto/arctic_network.proto` - Protocol definition
- `INFRASTRUCTURE.md` - Deployment guide
- `QUICKSTART.md` - Getting started
