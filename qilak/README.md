# Qilak

**"Qilak"** (Inuktitut: "sky" / "heaven" - ᕿᓚᒃ) — Server-side AI agent for the Qimmiit engine, providing cloud-based intelligence and strategic decision-making.

## Overview

Qilak is a Java-Quarkus microservice that serves as the server-side AI orchestrator for the Qimmiit engine. While **Nanook** (C++) orchestrates local operations, **Qilak** provides high-level AI-driven decisions and cloud-based compute capabilities.

### Philosophy

Like the sky above, Qilak provides:
- **Overarching Perspective**: Strategic, high-level decision making
- **Cloud-based Intelligence**: AI/ML-powered optimization and analysis
- **Distributed Compute**: Heavy computational tasks offloaded from clients
- **Tool Orchestration**: Uses Akutik tools (Java bindings) for data operations

## Architecture

```
┌─────────────────────────────────────────────────┐
│              Nanook (C++ Client)                │
│          Local Orchestration Agent              │
└─────────────────┬───────────────────────────────┘
                  │
                  │ gRPC Communication
                  ▼
┌─────────────────────────────────────────────────┐
│           Qilak (Java/Quarkus Server)           │  ← AI Decision Making
│                                                 │
│  ┌──────────────────────────────────────────┐   │
│  │        QilakAgent (Core)                 │   │
│  │  - AI-based optimization                │   │
│  │  - Performance analysis                 │   │
│  │  - Strategic resource allocation        │   │
│  └──────────────┬───────────────────────────┘   │
│                 │                               │
│                 │ uses                          │
│                 ▼                               │
│  ┌──────────────────────────────────────────┐   │
│  │    Akutik Tools (Java Bindings)         │   │
│  │  - AkutikStorage (SQLite)               │   │
│  │  - IgluStore (structured storage)       │   │
│  │  - Niqi (caching)                       │   │
│  └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
```

## Features

### AI-Powered Decisions
- **Asset Optimization**: ML-based recommendations for asset compression and LOD
- **Performance Analysis**: Real-time bottleneck detection and optimization suggestions
- **Shader Optimization**: AI-driven shader variant generation
- **LOD Recommendation**: Automatic Level-of-Detail strategy calculation

### Cloud Capabilities
- **Distributed Compute**: Offload heavy computations from client
- **Async Processing**: Long-running tasks with streaming responses
- **Caching**: Intelligent caching of AI decisions using Akutik tools
- **Scalability**: Horizontal scaling for multiple clients

### Integration
- **gRPC Communication**: High-performance binary protocol with Nanook
- **REST API**: HTTP endpoints for monitoring and control
- **Health Checks**: Built-in health monitoring and metrics
- **Hot Reload**: Quarkus dev mode for rapid development

## Technology Stack

- **Java 17**: Modern Java with records, pattern matching, and text blocks
- **Quarkus 3.8**: Cloud-native Java framework
- **gRPC**: High-performance RPC for Nanook ↔ Qilak communication
- **SQLite**: Lightweight database via Akutik bindings
- **Mutiny**: Reactive programming for async operations

## Building

### Prerequisites
- JDK 17 or later
- Maven 3.8+

### Build Commands

```bash
# Development mode (hot reload)
mvn quarkus:dev

# Package application
mvn clean package

# Run packaged application
java -jar target/quarkus-app/quarkus-run.jar

# Build native image
mvn package -Pnative
```

## Running

### Development Mode
```bash
mvn quarkus:dev
```

This starts Qilak with:
- gRPC server on port 50051
- HTTP API on port 8080
- Hot reload enabled
- Dev UI at http://localhost:8080/q/dev

### Production Mode
```bash
java -jar target/quarkus-app/quarkus-run.jar
```

## Configuration

Configuration is in [`application.yaml`](src/main/resources/application.yaml):

```yaml
qilak:
  name: "Qilak"
  grpc-port: 50051
  http-port: 8080

  storage:
    database-path: "qilak-data.db"
    cache-size-mb: 512
    cache-ttl-seconds: 3600

  ai:
    enabled: true
    model-path: "./models/qilak-optimizer.onnx"
    confidence-threshold: 0.7

  performance:
    worker-threads: 8
    max-concurrent-requests: 100
```

## API

### gRPC Service

#### MakeDecision
Request an AI-based decision from Qilak.

```protobuf
rpc MakeDecision(DecisionRequest) returns (DecisionResponse);
```

**Decision Types:**
- `OPTIMIZE_ASSET`: Get asset optimization recommendations
- `ANALYZE_PERFORMANCE`: Analyze performance bottlenecks
- `RECOMMEND_LOD`: Get LOD strategy recommendations
- `GENERATE_SHADER_VARIANT`: Request shader variant generation
- `COMPUTE_LIGHTING`: Offload lighting computation

#### MakeDecisionStream
Stream long-running decision results.

```protobuf
rpc MakeDecisionStream(DecisionRequest) returns (stream DecisionResponse);
```

#### HealthCheck
Check if Qilak is healthy and serving.

```protobuf
rpc HealthCheck(HealthCheckRequest) returns (HealthCheckResponse);
```

### REST API

#### Health Check
```bash
GET http://localhost:8080/health
```

#### Metrics
```bash
GET http://localhost:8080/metrics
```

## Usage Example

### From Nanook (C++)

```cpp
#include <grpcpp/grpcpp.h>
#include "qilak.grpc.pb.h"

// Create channel to Qilak
auto channel = grpc::CreateChannel(
    "localhost:50051",
    grpc::InsecureChannelCredentials()
);

auto stub = io::qimmiit::qilak::QilakService::NewStub(channel);

// Request asset optimization
io::qimmiit::qilak::DecisionRequest request;
request.set_type(io::qimmiit::qilak::DecisionRequest::OPTIMIZE_ASSET);
request.set_request_id("asset-123");
request.mutable_parameters()->insert({"asset_path", "models/character.gltf"});

// Make decision
io::qimmiit::qilak::DecisionResponse response;
grpc::ClientContext context;

grpc::Status status = stub->MakeDecision(&context, request, &response);

if (status.ok()) {
    std::cout << "Recommendation: " << response.recommendation() << std::endl;
    std::cout << "Confidence: " << response.confidence() << std::endl;
}
```

### Using Akutik Tools in Qilak

```java
@ApplicationScoped
public class AssetOptimizerModule implements AgentModule {

    @Inject
    AkutikStorage storage;

    @Override
    public boolean initialize(QilakAgent agent) {
        // Initialize Akutik storage
        storage.initialize("optimizer-cache.db");
        return true;
    }

    public DecisionResponse optimizeAsset(String assetPath) {
        // Business logic: Check cache first (using Akutik tool)
        Optional<String> cached = storage.getString("opt:" + assetPath);

        if (cached.isPresent()) {
            LOG.info("Cache hit for asset optimization");
            return DecisionResponse.fromJson(cached.get());
        }

        // Business logic: Run AI optimization
        var result = runAiOptimization(assetPath);

        // Store result using Akutik tool
        storage.putString("opt:" + assetPath, result.toJson());

        return result;
    }
}
```

## Design Principles

1. **Agent Orchestration**: Qilak orchestrates tools, tools have no business logic
2. **Akutik Tools**: Uses Akutik Java bindings for storage operations
3. **Reactive**: Async/non-blocking operations using Mutiny
4. **Cloud-Native**: Designed for containerized deployment
5. **Complementary to Nanook**: High-level AI decisions, not local operations

## Directory Structure

```
qilak/
├── src/
│   ├── main/
│   │   ├── java/io/qimmiit/qilak/
│   │   │   ├── core/              # Core agent classes
│   │   │   │   ├── QilakAgent.java
│   │   │   │   ├── AgentModule.java
│   │   │   │   └── AgentConfig.java
│   │   │   ├── services/          # gRPC services
│   │   │   │   └── QilakGrpcService.java
│   │   │   ├── tools/             # Akutik Java bindings
│   │   │   │   └── AkutikStorage.java
│   │   │   └── ai/                # AI modules
│   │   ├── proto/                 # Protocol buffers
│   │   │   └── qilak.proto
│   │   └── resources/
│   │       └── application.yaml
│   └── test/
│       └── java/
├── pom.xml
└── README.md
```

## Monitoring

### Health Endpoints
- `/health`: Overall health status
- `/health/live`: Liveness probe
- `/health/ready`: Readiness probe

### Metrics
- `/metrics`: Prometheus metrics
- gRPC metrics included
- JVM metrics included

## Development

### Running Tests
```bash
mvn test
```

### Dev UI
When running in dev mode, access the Dev UI at:
```
http://localhost:8080/q/dev
```

## Docker

### Build Image
```bash
docker build -f src/main/docker/Dockerfile.jvm -t qilak:latest .
```

### Run Container
```bash
docker run -p 8080:8080 -p 50051:50051 qilak:latest
```

## Integration with Qimmiit

Qilak is part of the Qimmiit engine ecosystem:

- **Nanook** (C++): Local orchestration agent
- **Akutik** (C++/Java): Shared tool library
- **Qilak** (Java): Server-side AI agent
- **Siku** (Tool): Additional tooling

## License

[License information needed]

---

*"Like the sky above, Qilak provides perspective and clarity from above."*
