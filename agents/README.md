# Qimmiit Agents

Independent services that provide specialized functionality to Qimmiit applications.

## Overview

Agents are **completely independent executables** that communicate via gRPC or HTTP. They have zero dependencies on the Qimmiit engine and can be written in any language.

## Quick Start

### Using an Agent (Client)

```cpp
#include <grpcpp/grpcpp.h>
#include "umiak.grpc.pb.h"

// Connect to agent
auto channel = grpc::CreateChannel("localhost:50060",
                                  grpc::InsecureChannelCredentials());
auto umiak = cryo::agents::umiak::UmiakAgent::NewStub(channel);

// Use agent
cryo::agents::umiak::EntityQuery query;
query.set_name_pattern("suzanne-in-box");

cryo::agents::umiak::EntityResult result;
grpc::ClientContext ctx;
umiak->FindEntity(&ctx, query, &result);

if (result.found()) {
    // Move entity to [2, 2, 2]
    cryo::agents::umiak::MoveEntityRequest moveReq;
    moveReq.mutable_entity()->set_node_id(result.entity().node_id());
    moveReq.mutable_position()->set_x(2.0f);
    moveReq.mutable_position()->set_y(2.0f);
    moveReq.mutable_position()->set_z(2.0f);

    cryo::agents::umiak::OperationResult moveResult;
    grpc::ClientContext moveCtx;
    umiak->MoveEntity(&moveCtx, moveReq, &moveResult);
}
```

### Creating an Agent

```cpp
#include <agent-shared/BaseAgent.hpp>
#include <grpcpp/grpcpp.h>

class MyAgent : public cryo::agents::BaseAgent {
public:
    MyAgent() : BaseAgent("my-agent", "1.0.0", "grpc") {}

protected:
    bool onStart() override {
        // Start gRPC server
        return true;
    }

    void onShutdown() override {
        // Stop server
    }
};

int main() {
    MyAgent agent;
    agent.initialize({{"port", "50051"}});
    agent.start();

    // Wait for shutdown signal...

    agent.shutdown();
    return 0;
}
```

## Directory Structure

```
agents/
├── shared/                     # Common interfaces and base classes
│   ├── include/
│   │   ├── IAgentModule.hpp   # Base interface
│   │   └── BaseAgent.hpp      # Base implementation
│   ├── proto/
│   │   └── agent_common.proto # Standard health/metrics
│   └── README.md
│
├── umiak-agent/                # High-level scene manipulation
│   ├── src/
│   ├── proto/umiak.proto
│   └── README.md
│
├── AGENT_GUIDE.md             # Complete guide to building agents
├── AGENT_TYPES.md             # Agent hierarchy and examples
└── README.md                  # This file
```

## Agent Hierarchy

### Low-Level Service Agents
Generic computation and I/O services:
- Compute Agent
- Storage Agent
- HTTP Client Agent
- Database Agent

### Mid-Level Processing Agents
Domain-specific processing:
- Asset Optimizer Agent
- Format Converter Agent
- LOD Generator Agent
- Lightmap Baker Agent

### High-Level Semantic Agents
Scene understanding and intelligent operations:
- **Umiak Agent**: Scene manipulation and queries
- AI Director Agent: Game logic
- Content Analyzer Agent: Scene analysis
- Layout Agent: Intelligent placement

## Example: High-Level Semantic Operation

```cpp
// Find "suzanne-in-box" entity and move it to [2, 2, 2]
auto umiak = UmiakAgent::NewStub(channel);

// 1. Find the entity
EntityQuery query;
query.set_name_pattern("suzanne-in-box");

EntityResult result;
grpc::ClientContext findCtx;
umiak->FindEntity(&findCtx, query, &result);

// 2. Move it
if (result.found()) {
    MoveEntityRequest moveReq;
    moveReq.mutable_entity()->set_node_id(result.entity().node_id());
    moveReq.mutable_position()->set_x(2.0f);
    moveReq.mutable_position()->set_y(2.0f);
    moveReq.mutable_position()->set_z(2.0f);

    OperationResult moveResult;
    grpc::ClientContext moveCtx;
    umiak->MoveEntity(&moveCtx, moveReq, &moveResult);

    std::cout << "Moved suzanne to [2, 2, 2]" << std::endl;
}
```

## Key Principles

1. **Independence**: Agents are separate executables, no engine dependencies
2. **Language Agnostic**: Can be written in C++, Python, Go, Rust, etc.
3. **Well-Defined Contracts**: Communication via Protocol Buffers
4. **Process Isolation**: Agents run in separate processes
5. **Scalability**: Scale agents independently
6. **Composability**: High-level agents can use low-level agents

## Building Agents

### C++ Agent

```bash
cd agents/my-agent
mkdir build && cd build
cmake ..
make
./my-agent --port 50051
```

### Python Agent

```bash
cd agents/my-agent
pip install grpcio grpcio-tools
python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. proto/*.proto
python server.py
```

### Go Agent

```bash
cd agents/my-agent
go mod init my-agent
protoc --go_out=. --go-grpc_out=. proto/*.proto
go build -o my-agent
./my-agent
```

## Deployment

### Docker
```bash
docker build -t my-agent .
docker run -p 50051:50051 my-agent
```

### Docker Compose
```yaml
services:
  umiak:
    build: ./agents/umiak-agent
    ports:
      - "50060:50060"

  asset-optimizer:
    build: ./agents/asset-optimizer-agent
    ports:
      - "50061:50061"
```

### Kubernetes
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: umiak-agent
spec:
  replicas: 3
  template:
    spec:
      containers:
      - name: umiak
        image: umiak-agent:1.0.0
        ports:
        - containerPort: 50060
```

## Health Checks

All agents implement standard health checks:

```bash
# Check agent health
grpcurl -plaintext localhost:50060 cryo.agents.common.AgentHealth/Check

# Get agent metadata
grpcurl -plaintext localhost:50060 cryo.agents.common.AgentHealth/GetMetadata

# Get agent metrics
grpcurl -plaintext localhost:50060 cryo.agents.common.AgentHealth/GetMetrics
```

## Available Agents

| Agent | Port | Purpose | Language |
|-------|------|---------|----------|
| Umiak | 50060 | Scene manipulation | C++ |
| Asset Optimizer | 50061 | Asset optimization | C++ |
| Compute | 50062 | Generic computation | C++ |
| HTTP Client | 50063 | HTTP requests | C++ |

## Documentation

- **[AGENT_GUIDE.md](AGENT_GUIDE.md)** - Complete guide to building agents
- **[AGENT_TYPES.md](AGENT_TYPES.md)** - Agent hierarchy and examples
- **[shared/README.md](shared/README.md)** - Using the shared library
- **[umiak-agent/README.md](umiak-agent/README.md)** - Umiak agent documentation

## Use Cases

### Game Development
```cpp
// Find spawn point and move player
auto spawn = umiak->FindEntity(EntityQuery{tags: ["spawn_point"]});
umiak->MoveEntity(playerId, spawn.position());
```

### Level Editing
```cpp
// Move all lights up by 5 units
auto lights = umiak->ListEntities(EntityQuery{type: "Light"});
umiak->BatchMove(lights, Vec3{0, 5, 0}, relative=true);
```

### Content Validation
```cpp
// Analyze scene for issues
auto analysis = umiak->AnalyzeScene(sceneData);
for (const auto& warning : analysis.warnings()) {
    std::cerr << "Warning: " << warning << std::endl;
}
```

### Procedural Generation
```cpp
// Place collectibles on platforms
auto platforms = umiak->FindBySemantics(SemanticQuery{tags: ["platform"]});
for (const auto& platform : platforms.entities()) {
    Vec3 pos = platform.position();
    pos.y += 2.0f;
    umiak->MoveEntity(collectibleId, pos);
}
```

## Agent Composition

Agents can work together:

```cpp
// High-level agent delegates to low-level agents
auto moveResult = umiak->MoveEntity(entityId, newPosition);

if (moveResult.success()) {
    // Optimize the moved entity
    assetOptimizer->OptimizeEntity(entityId);

    // Update LODs
    lodGenerator->UpdateLODs(entityId);

    // Save scene
    storage->SaveScene(sceneId);
}
```

## Testing

```bash
# Unit test agent logic
./build/my-agent-test

# Integration test gRPC interface
grpcurl -plaintext -d '{"name": "test"}' localhost:50051 MyAgent/TestMethod

# Load test
ghz --insecure --proto proto/my-agent.proto --call MyAgent/TestMethod \
    -d '{"name": "test"}' -n 10000 -c 100 localhost:50051
```

## Contributing

When creating new agents:

1. Use `agents/shared` for common functionality
2. Implement standard health checks
3. Document your agent's capabilities
4. Provide usage examples
5. Include Dockerfile for deployment
6. Write integration tests

## License

[License information]

---

*"Agents are independent specialists, each bringing unique capabilities to the Qimmiit ecosystem."*
