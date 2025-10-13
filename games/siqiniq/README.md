# Siqiniq-001 Game Server

**Siqiniq-001** is a cloud-native **Render Stream Server** built with Quarkus. It implements gRPC services from arctic-formats for real-time rendering stream and advanced shader pipeline management.

> **Name:** Siqiniq (ᓯᕿᓂᖅ) - Inuktitut word meaning "Sun"

## What is Siqiniq-001?

Siqiniq-001 is **NOT** a traditional game server with business logic. Instead, it is a **Render Stream Server** that:

✅ **Implements gRPC Services from arctic-formats:**
- `RenderStreamService` - Bidirectional rendering updates
- `ShaderGraphStreamingService` - Advanced shader pipelines with adaptive LOD

✅ **Provides Admin/Metrics APIs:**
- Connection monitoring
- Performance metrics
- Frame statistics

❌ **Does NOT provide:**
- Game logic (worlds, items, characters)
- Traditional REST CRUD APIs
- Game state management

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Siqiniq-001 Server                      │
│  ┌───────────────────────────────────────────────────────┐ │
│  │         RenderStreamService (gRPC Port 9000)          │ │
│  │  • StreamRenderingUpdates (ClientEvent ↔ RenderEvent) │ │
│  │  • GetSceneSnapshot (SceneSnapshotRequest → Snapshot) │ │
│  └───────────────────────────────────────────────────────┘ │
│  ┌───────────────────────────────────────────────────────┐ │
│  │    ShaderGraphStreamingService (gRPC Port 9000)       │ │
│  │  • StreamShaderGraphs (Command ↔ Response)            │ │
│  │  • LoadGraph (LoadCmd → Response)                     │ │
│  │  • MonitorPerformance (Void → PerformanceReport)      │ │
│  └───────────────────────────────────────────────────────┘ │
│  ┌───────────────────────────────────────────────────────┐ │
│  │       AdminMetricsResource (REST Port 8080)           │ │
│  │  • /api/admin/metrics/summary                         │ │
│  │  • /api/admin/metrics/connections                     │ │
│  │  • /api/admin/metrics/performance                     │ │
│  │  • /api/admin/metrics/frame-stats                     │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
           ↓                    ↓                    ↓
    ┌──────────┐         ┌──────────┐         ┌──────────┐
    │ MariaDB  │         │  Redis   │         │   Logs   │
    └──────────┘         └──────────┘         └──────────┘
```

## Quick Start

### Prerequisites

- Java 21+
- Maven 3.9+
- Docker & Docker Compose (for infrastructure)
- arctic-formats submodule initialized

### 1. Build

```bash
# JVM mode (fast development)
./mvnw clean package
java -jar target/quarkus-app/quarkus-run.jar

# Native mode (fastest startup, production)
./mvnw clean package -Pnative
./target/siqiniq-001-1.0.0-SNAPSHOT-runner
```

### 2. Run with Docker

```bash
cd docker

# Basic (server + MariaDB + Redis)
docker-compose up -d

# With admin tools (phpMyAdmin, Redis Commander)
docker-compose --profile admin up -d

# With monitoring (Prometheus, Grafana)
docker-compose --profile monitoring up -d

# Everything
docker-compose --profile admin --profile monitoring up -d
```

### 3. Deploy to Kubernetes

```bash
kubectl apply -f docker/k8s/
kubectl get pods -n siqiniq-001
```

## Services and Ports

| Service | Protocol | Port | Endpoint | Purpose |
|---------|----------|------|----------|---------|
| RenderStream | gRPC | 9000 | `/RenderStreamService/*` | Rendering updates |
| ShaderGraph | gRPC | 9000 | `/ShaderGraphStreamingService/*` | Shader pipelines |
| Admin API | REST | 8080 | `/api/admin/metrics/*` | Monitoring |
| Health | REST | 8080 | `/q/health` | Health checks |
| Metrics | REST | 8080 | `/q/metrics` | Prometheus |
| OpenAPI | REST | 8080 | `/swagger-ui` | API docs |

## Key Features

### 1. RenderStreamService (Core)

**Real-time bidirectional rendering updates**

**Client → Server:**
- Input events (mouse, keyboard, gamepad)
- Camera controls
- Entity commands (move, rotate, scale)
- Scene commands (load/unload, spawn)

**Server → Client:**
- Scene loaded with resources
- Entity spawned/updated/destroyed
- Camera updates
- Light updates
- Material updates
- Frame statistics

**Usage:**
```bash
# Test with grpcurl
grpcurl -plaintext localhost:9000 list RenderStreamService
```

### 2. ShaderGraphStreamingService (Advanced)

**Advanced shader pipeline with adaptive LOD**

**Features:**
- Load complete shader graphs
- Hot-reload individual shader nodes
- Automatic quality adaptation based on FPS
- Manual LOD level control (0-3)
- Performance monitoring and telemetry

**LOD Levels:**
- **Level 0** (Highest): Full PBR with IBL (~8-10ms)
- **Level 1** (High): PBR without IBL (~5-6ms)
- **Level 2** (Medium): Simplified with specular (~3-4ms)
- **Level 3** (Low): Basic Lambert (~1-2ms)

**Automatic Adaptation:**
- Frame time > 16.67ms → Drop LOD
- Frame time < 12ms → Increase LOD

**Usage:**
```bash
# Monitor performance
grpcurl -plaintext localhost:9000 \
  ShaderGraphStreamingService/MonitorPerformance
```

### 3. Admin Metrics API

**REST endpoints for monitoring**

```bash
# Summary
curl http://localhost:8080/api/admin/metrics/summary

# Connections
curl http://localhost:8080/api/admin/metrics/connections

# Performance
curl http://localhost:8080/api/admin/metrics/performance

# Frame stats
curl http://localhost:8080/api/admin/metrics/frame-stats
```

## Configuration

### application.properties

```properties
# gRPC
quarkus.grpc.server.port=9000
quarkus.grpc.server.max-inbound-message-size=4194304

# Database (MariaDB)
quarkus.datasource.jdbc.url=jdbc:mariadb://localhost:3306/siqiniq
quarkus.datasource.username=siqiniq
quarkus.datasource.password=siqiniq

# Redis
quarkus.redis.hosts=redis://localhost:6379

# Shader LOD
siqiniq.shader.lod.auto-adapt=true
siqiniq.shader.lod.target-frame-time-ms=16.67
```

## Development

### Project Structure

```
siqiniq-001/
├── src/main/java/io/qimmiit/siqiniq/
│   ├── server/
│   │   ├── render/                    # RenderStreamService
│   │   │   ├── RenderStreamServiceImpl.java
│   │   │   └── RenderStreamManager.java
│   │   └── shader/                    # ShaderGraphStreamingService
│   │       ├── ShaderGraphStreamingServiceImpl.java
│   │       ├── ShaderGraphManager.java
│   │       └── AdaptiveLODSystem.java
│   └── cms/api/
│       └── AdminMetricsResource.java  # Admin REST API
├── src/main/proto/                    # Proto definitions
├── docker/                            # Docker Compose & K8s
├── ARCHITECTURE.md                    # Architecture overview
├── SERVICES.md                        # Service documentation
├── GRPC_IMPLEMENTATION.md             # Implementation guide
└── INFRASTRUCTURE.md                  # Infrastructure setup
```

### Dev Mode

```bash
# Hot reload enabled
./mvnw quarkus:dev

# Access Dev UI
http://localhost:8080/q/dev
```

### Testing

```bash
# Unit tests
./mvnw test

# Integration tests
./mvnw verify

# With coverage
./mvnw verify -Pcoverage
```

## Monitoring

### Health Checks

```bash
curl http://localhost:8080/q/health
curl http://localhost:8080/q/health/live
curl http://localhost:8080/q/health/ready
```

### Prometheus Metrics

```bash
curl http://localhost:8080/q/metrics
```

**Key Metrics:**
- `siqiniq_render_stream_events_sent_total`
- `siqiniq_render_stream_active_connections`
- `siqiniq_shader_graphs_loaded_total`
- `siqiniq_shader_lod_changes_total`

### Grafana Dashboards

Access Grafana at http://localhost:3000 (admin/admin)

## Client Integration

### C++ Client (Qimmiit Renderer)

```cpp
#include <grpcpp/grpcpp.h>
#include "arctic_network.grpc.pb.h"

// Connect to server
auto channel = grpc::CreateChannel("localhost:9000",
    grpc::InsecureChannelCredentials());
auto stub = RenderStreamService::NewStub(channel);

// Start streaming
grpc::ClientContext context;
auto stream = stub->StreamRenderingUpdates(&context);

// Send events
ClientEvent event;
event.mutable_camera_control()->mutable_movement()->set_x(1.0f);
stream->Write(event);

// Receive updates
RenderEvent render_event;
while (stream->Read(&render_event)) {
    // Process render events
}
```

## Performance

### Startup Times

| Mode | Startup | Memory |
|------|---------|--------|
| JVM | ~3s | ~200MB |
| Native | ~0.08s | ~50MB |

### Throughput

- **RenderStreamService:** 60+ FPS stream per client
- **ShaderGraphStreamingService:** Real-time node updates
- **Adaptive LOD:** Automatic quality adjustment within 100ms

### Scaling

- **Horizontal:** Auto-scaling with HPA (2-10 replicas)
- **Vertical:** Configurable CPU/memory limits
- **Load Balancing:** Multiple server instances behind LB

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture and design
- [SERVICES.md](SERVICES.md) - Detailed service documentation
- [GRPC_IMPLEMENTATION.md](GRPC_IMPLEMENTATION.md) - Implementation guide
- [INFRASTRUCTURE.md](INFRASTRUCTURE.md) - Infrastructure and deployment

## Technology Stack

- **Framework:** Quarkus 3.17.0
- **Language:** Java 21
- **Database:** MariaDB 11.4
- **Cache:** Redis 7
- **Protocol:** gRPC + Protocol Buffers
- **Container:** Docker + Kubernetes
- **Monitoring:** Prometheus + Grafana
- **Build:** Maven

## Contributing

This is part of the Qimmiit game engine ecosystem. For issues or contributions:

1. Check existing documentation
2. Test with arctic-formats integration
3. Ensure all tests pass
4. Follow code style guidelines

## License

Part of the Qimmiit Engine project.

---

**Status:** ✅ Implementation complete, pending arctic-formats integration for proto generation

**Version:** 1.0.0-SNAPSHOT
