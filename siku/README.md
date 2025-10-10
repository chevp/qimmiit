# Siku

**"Siku"** (Inuktitut: "Eis" / "Ice") — The foundational platform and communication layer that enables the Qimmiit engine ecosystem to function.

## Concept Overview

Just as ice provides the stable foundation for polar bears (Nanook) and dog sled teams (Qimmiit) to traverse vast Arctic landscapes, **Siku** provides the underlying infrastructure and communication substrate that enables distributed engine components to work together seamlessly.

### Metaphor

In the Arctic:
- **Ice (Siku)** forms the foundation — the platform that connects everything
- **Polar Bear (Nanook)** orchestrates and coordinates on top of the ice
- **Dog Sled Team (Qimmiit)** travels across the ice, each dog working independently but together

In Qimmiit Engine:
- **Siku** provides the communication, discovery, and platform services
- **Nanook** orchestrates module coordination using Siku's infrastructure
- **Qimmiit modules** communicate through Siku's channels

## Purpose

Siku serves as the **foundational infrastructure layer** that Nanook and other Qimmiit components build upon:

### Core Responsibilities

1. **Service Discovery** — Finding and connecting distributed services
2. **Message Transport** — Reliable message passing between components
3. **State Synchronization** — Keeping distributed state consistent
4. **Health Monitoring** — Tracking component health and availability
5. **Configuration Distribution** — Propagating configuration across services
6. **Resource Registry** — Central registry of available resources and capabilities

## Architecture

### Siku as Infrastructure

```
┌─────────────────────────────────────────────────────┐
│                   Qimmiit Modules                   │
│        (Renderer, Cache, Asset, HTTP, etc.)         │
└───────────────────┬─────────────────────────────────┘
                    │
┌───────────────────▼─────────────────────────────────┐
│                      Nanook                          │
│            (Module Orchestration Layer)              │
└───────────────────┬─────────────────────────────────┘
                    │
┌───────────────────▼─────────────────────────────────┐
│                       Siku                           │
│          (Infrastructure & Platform Layer)           │
│  ┌────────────┐  ┌──────────┐  ┌─────────────────┐ │
│  │  Service   │  │ Message  │  │     State       │ │
│  │ Discovery  │  │ Transport│  │ Synchronization │ │
│  └────────────┘  └──────────┘  └─────────────────┘ │
│  ┌────────────┐  ┌──────────┐  ┌─────────────────┐ │
│  │   Health   │  │  Config  │  │    Resource     │ │
│  │ Monitoring │  │   Dist.  │  │    Registry     │ │
│  └────────────┘  └──────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────┘
```

## Key Features

### 1. Service Discovery

**Problem**: Nanook modules need to find each other, especially in distributed scenarios.

**Solution**: Siku provides a service registry and discovery mechanism.

```cpp
// Service Registration
SikuServiceRegistry registry;
registry.registerService("renderer", "localhost:8080", {
    .type = ServiceType::RENDERER,
    .capabilities = {"vulkan", "raytracing"},
    .status = ServiceStatus::HEALTHY
});

// Service Discovery
auto rendererServices = registry.findServices("renderer");
auto rtxCapable = registry.findByCapability("raytracing");
```

### 2. Message Transport

**Problem**: Modules need reliable communication across process boundaries.

**Solution**: Siku provides multiple transport mechanisms (local, gRPC, WebSocket).

```cpp
// Create transport channel
SikuChannel channel = siku.createChannel("renderer-cache");

// Send message
channel.send(Message{
    .type = MessageType::CACHE_REQUEST,
    .source = "renderer",
    .target = "cache",
    .payload = SceneId{12345}
});

// Subscribe to messages
channel.subscribe([](const Message& msg) {
    // Handle message
});
```

### 3. State Synchronization

**Problem**: Distributed modules need consistent view of shared state.

**Solution**: Siku provides distributed state management with conflict resolution.

```cpp
// Shared state
SikuState state = siku.getState("scene.current");

// Update state
state.set("cameraPosition", Vector3{0, 5, 10});
state.set("renderMode", "pbr");

// Subscribe to changes
state.onChange([](const std::string& key, const Value& value) {
    // React to state changes
});
```

### 4. Health Monitoring

**Problem**: System needs to know if services are alive and responsive.

**Solution**: Siku monitors service health with heartbeats and health checks.

```cpp
// Register health check
siku.registerHealthCheck("renderer", []() -> HealthStatus {
    return {
        .status = Status::HEALTHY,
        .details = {
            {"fps", "60"},
            {"memory_mb", "512"}
        }
    };
});

// Monitor health
siku.onHealthChange([](const std::string& service, HealthStatus status) {
    if (status == Status::UNHEALTHY) {
        // Handle service degradation
    }
});
```

### 5. Configuration Distribution

**Problem**: Configuration changes need to propagate to all services.

**Solution**: Siku provides centralized configuration with push updates.

```cpp
// Subscribe to config changes
siku.watchConfig("renderer.quality", [](const ConfigValue& value) {
    updateRenderQuality(value.toString());
});

// Update config (propagates to all watchers)
siku.setConfig("renderer.quality", "ultra");
```

### 6. Resource Registry

**Problem**: Components need to know what resources are available.

**Solution**: Siku maintains a registry of capabilities and resources.

```cpp
// Register capabilities
siku.registerCapability("gpu", {
    .vendor = "NVIDIA",
    .model = "RTX 4090",
    .memory_mb = 24576,
    .features = {"raytracing", "dlss", "mesh_shaders"}
});

// Query capabilities
auto gpus = siku.queryCapabilities("gpu");
if (gpus.hasFeature("raytracing")) {
    enableRaytracing();
}
```

## Implementation Approach

### Phase 1: Core Infrastructure (Foundation)

- Service registry and discovery
- Local message transport (in-process)
- Basic health monitoring
- Configuration store

### Phase 2: Distribution (Ice Sheet Formation)

- gRPC-based remote transport
- Distributed state synchronization
- Advanced health checking with metrics
- Configuration distribution with versioning

### Phase 3: Resilience (Ice Stability)

- Automatic failover and recovery
- Circuit breakers for failing services
- Load balancing and routing
- Distributed tracing and observability

### Phase 4: Advanced Features (Arctic Infrastructure)

- Service mesh capabilities
- Policy-based routing
- Security and authentication
- Multi-cluster support

## Technology Stack

### Core Technologies

- **C++17** for core implementation
- **gRPC** for remote procedure calls
- **Protocol Buffers** for serialization
- **etcd** or **Consul** for distributed configuration (optional)
- **ZeroMQ** or **nanomsg** for high-performance messaging
- **Redis** for distributed state caching (optional)

### Integration

- **Nanook** uses Siku for inter-module communication
- **Cryo Studio Server** uses Siku for client-server coordination
- **Compute Engine** uses Siku for job distribution
- **Asset Pipeline** uses Siku for distributed processing

## Example: Nanook + Siku Integration

```cpp
// Initialize Siku first (foundation)
auto& siku = Siku::getInstance();
SikuConfig sikuConfig{
    .mode = SikuMode::DISTRIBUTED,
    .discoveryEndpoint = "localhost:2379",
    .transportType = TransportType::GRPC
};
siku.initialize(sikuConfig);

// Initialize Nanook on top of Siku
auto& nanook = CentralAgent::getInstance();
EngineConfig nanookConfig{
    .configFile = "arctic.config.xml",
    .communicationLayer = &siku  // Nanook uses Siku
};
nanook.initialize(nanookConfig);

// Register modules (they automatically use Siku for communication)
nanook.registerModule(std::make_shared<RendererModule>());
nanook.registerModule(std::make_shared<CacheModule>());

// Modules communicate through Siku
// RendererModule → Siku → CacheModule
```

## Benefits

### For Nanook

- **Decoupling**: Modules don't need direct references to each other
- **Distribution**: Modules can run in separate processes or machines
- **Resilience**: Automatic handling of service failures
- **Scalability**: Easy horizontal scaling of services

### For Developers

- **Simplicity**: Unified API for local and remote communication
- **Flexibility**: Choose appropriate transport for each scenario
- **Observability**: Built-in tracing and monitoring
- **Testing**: Easy to mock and test distributed scenarios

### For Operations

- **Monitoring**: Centralized view of system health
- **Configuration**: Dynamic configuration without restarts
- **Deployment**: Independent deployment of services
- **Debugging**: Distributed tracing and logging

## Relationship to Other Components

### Siku ↔ Nanook

- **Siku**: "How do modules communicate?"
- **Nanook**: "How do modules coordinate?"
- Nanook uses Siku as its communication substrate

### Siku ↔ Qimmiit Modules

- **Siku**: Provides the transport layer
- **Modules**: Use Siku for discovery and messaging
- Transparent whether modules are local or remote

### Siku ↔ Arctic Graphics Engine

- **Siku**: Enables distributed rendering pipeline
- **Arctic**: Focuses on graphics and rendering
- Can distribute workload across multiple nodes

## File Structure (Proposed)

```
siku/
├── src/
│   ├── core/
│   │   ├── SikuCore.hpp/cpp           # Main Siku interface
│   │   ├── ServiceRegistry.hpp/cpp    # Service discovery
│   │   ├── MessageTransport.hpp/cpp   # Message passing
│   │   └── StateSync.hpp/cpp          # State synchronization
│   ├── transport/
│   │   ├── LocalTransport.hpp/cpp     # In-process transport
│   │   ├── GrpcTransport.hpp/cpp      # gRPC transport
│   │   └── ZmqTransport.hpp/cpp       # ZeroMQ transport
│   ├── health/
│   │   ├── HealthMonitor.hpp/cpp      # Health checking
│   │   └── Metrics.hpp/cpp            # Metrics collection
│   └── config/
│       ├── ConfigStore.hpp/cpp        # Configuration storage
│       └── ConfigWatcher.hpp/cpp      # Configuration watching
├── proto/
│   ├── siku_service.proto             # Service definitions
│   └── siku_messages.proto            # Message types
├── tests/
│   ├── unit/
│   └── integration/
├── examples/
│   ├── simple/                        # Simple local example
│   └── distributed/                   # Distributed example
├── CMakeLists.txt
└── README.md                          # This file
```

## Use Cases

### 1. Local Development
Single machine, all modules in one process, Siku provides uniform API.

### 2. Distributed Studio
Renderer on GPU machine, cache on storage server, UI on client machine.

### 3. Cloud Rendering
Multiple render nodes coordinated through Siku, load balanced automatically.

### 4. Development Tools
Editor, shader compiler, asset processor all communicate through Siku.

## Next Steps

1. **Design Document**: Detailed API design for Siku core
2. **Prototype**: Simple local transport implementation
3. **Integration**: Modify Nanook to use Siku for EventBus
4. **Testing**: Benchmark local vs remote performance
5. **Documentation**: Complete API documentation

## Philosophy

> "Like ice forms the foundation for Arctic life, Siku forms the foundation for the Qimmiit ecosystem. It must be:
> - **Solid**: Reliable and stable
> - **Transparent**: Easy to understand and use
> - **Expansive**: Able to connect distant components
> - **Adaptive**: Handle changing conditions (failures, scaling)
>
> Just as ice can be thin or thick, smooth or rough, Siku adapts to different deployment scenarios while maintaining a consistent surface for the layers above."

## References

- [gRPC](https://grpc.io/) - RPC framework
- [etcd](https://etcd.io/) - Distributed configuration
- [Consul](https://www.consul.io/) - Service mesh
- [ZeroMQ](https://zeromq.org/) - High-performance messaging
- [Microservices Patterns](https://microservices.io/patterns/) - Architecture patterns

---

**Status**: Concept / Design Phase
**Dependencies**: None (foundational layer)
**Dependents**: Nanook, all Qimmiit modules
**Priority**: High (foundational infrastructure)

*"The ice connects all. Without the ice, there is no journey."*
