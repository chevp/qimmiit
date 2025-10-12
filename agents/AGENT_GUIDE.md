# Agent Architecture Guide

## Overview

In the Qimmiit ecosystem, an **Agent** is a completely independent program that accomplishes a specific task with well-defined input/output interfaces. Agents are **separate executables** that run independently from the core engine and communicate through Protocol Buffers (gRPC) or HTTP APIs.

### Key Characteristics

- **Independent Executables**: Agents are standalone programs, not modules
- **Process Isolation**: Run in separate processes with their own lifecycle
- **Well-Defined Interfaces**: Communication via gRPC or HTTP only
- **No Engine Dependencies**: Agents do not depend on or link against the core engine
- **Task-Specific**: Each agent focuses on a specific domain or capability
- **Optional Connectivity**: Agents may operate standalone or require network connections
- **Reusable**: Can be utilized by any application, not just Qimmiit apps

## Agent vs Module

Understanding the distinction is crucial:

| Aspect | Module | Agent |
|--------|--------|-------|
| **Execution** | Runs inside engine process | Separate executable/process |
| **Lifecycle** | Managed by CentralAgent | Independent, self-managed |
| **Dependencies** | Links against engine libs | No engine dependencies |
| **Communication** | Direct function calls | gRPC/HTTP only |
| **Deployment** | Part of application binary | Separate deployment unit |
| **Language** | Must be C++ | Any language (C++, Go, Python, etc.) |
| **Examples** | RendererModule, CacheModule | compute-agent, studio-server-agent |

### When to Build an Agent

Choose an agent when:
- You need complete independence from the engine
- You want process isolation for stability or security
- The service should scale independently
- You want to use a different programming language
- Multiple applications (not just Qimmiit) need this functionality
- You need to deploy/update independently

### When to Build a Module

Choose a module when:
- Tight integration with engine is required
- Performance demands direct memory access
- The feature is core to engine operation
- You need frame-synchronized execution

## Agent Architecture

### Communication Patterns

Agents communicate **exclusively** through network protocols:

#### 1. gRPC (Recommended)

Strongly-typed, high-performance RPC framework:

```protobuf
// compute_agent.proto
syntax = "proto3";

package cryo.agents;

service ComputeAgent {
    rpc ProcessTask(TaskRequest) returns (TaskResponse);
    rpc StreamResults(TaskRequest) returns (stream ResultChunk);
    rpc Healthcheck(Empty) returns (HealthStatus);
}

message TaskRequest {
    string task_id = 1;
    bytes input_data = 2;
    map<string, string> parameters = 3;
}

message TaskResponse {
    string task_id = 1;
    bool success = 2;
    bytes output_data = 3;
    string error_message = 4;
}
```

**Benefits:**
- Strong typing via protobuf
- Efficient binary serialization
- Streaming support
- Language-agnostic (C++, Go, Python, Java, etc.)

#### 2. HTTP/REST

Simple, universal HTTP-based communication:

```json
// POST /api/v1/process
{
    "task_id": "uuid-here",
    "input_data": "base64-encoded-data",
    "parameters": {
        "quality": "high"
    }
}

// Response
{
    "task_id": "uuid-here",
    "success": true,
    "output_data": "base64-encoded-result"
}
```

**Benefits:**
- Universal protocol
- Easy debugging (curl, postman)
- Firewall-friendly
- Simple to implement

### Agent Structure

```
agents/
├── compute-agent/              # Independent compute service
│   ├── src/
│   │   └── main.cpp           # Entry point (no engine deps!)
│   ├── proto/
│   │   └── compute.proto      # Service definition
│   ├── CMakeLists.txt         # Only depends on gRPC/protobuf
│   ├── Dockerfile             # Containerization
│   └── README.md
├── studio-server-agent/        # Independent HTTP service
│   ├── src/
│   │   └── main.cpp           # Entry point
│   ├── proto/
│   │   └── studio_api.proto
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   └── README.md
└── AGENT_GUIDE.md             # This file
```

## Implementation Guide

### 1. Designing Your Agent

Define your agent's contract:

```cpp
/**
 * Agent: Scene Optimizer
 *
 * Type: Independent gRPC Service
 * Language: C++
 * Port: 50051
 *
 * Purpose: Optimize glTF scenes for runtime rendering
 *
 * Input (gRPC):
 *   - OptimizeRequest: scene path, settings
 *
 * Output (gRPC):
 *   - OptimizeResponse: optimized scene path, stats
 *
 * Dependencies: NONE (no engine libraries)
 * Can run standalone: YES
 */
```

### 2. Define the Service Contract

```protobuf
// agents/scene-optimizer-agent/proto/optimizer.proto
syntax = "proto3";

package cryo.agents.optimizer;

service SceneOptimizer {
    // Optimize a scene file
    rpc OptimizeScene(OptimizeRequest) returns (OptimizeResponse);

    // Health check
    rpc Healthcheck(Empty) returns (HealthStatus);
}

message OptimizeRequest {
    string scene_path = 1;
    OptimizationSettings settings = 2;
}

message OptimizationSettings {
    int32 lod_levels = 1;
    bool compress_textures = 2;
    bool merge_meshes = 3;
}

message OptimizeResponse {
    bool success = 1;
    string optimized_path = 2;
    string error_message = 3;
    OptimizationStats stats = 4;
}

message OptimizationStats {
    int32 original_vertices = 1;
    int32 optimized_vertices = 2;
    float reduction_percentage = 3;
}

message Empty {}

message HealthStatus {
    bool healthy = 1;
    string version = 2;
}
```

### 3. Implement the Agent (C++)

#### CMakeLists.txt - No Engine Dependencies!

```cmake
# agents/scene-optimizer-agent/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(scene-optimizer-agent)

set(CMAKE_CXX_STANDARD 17)

# Only depend on gRPC and protobuf - NO ENGINE LIBRARIES!
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Generate protobuf/gRPC code
set(PROTO_FILES proto/optimizer.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTO_FILES})

# Build the agent executable
add_executable(scene-optimizer-agent
    src/main.cpp
    src/optimizer_service.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

target_include_directories(scene-optimizer-agent PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(scene-optimizer-agent PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
    # NO ENGINE LIBRARIES - completely independent!
)
```

#### main.cpp - Standalone Executable

```cpp
// agents/scene-optimizer-agent/src/main.cpp
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "optimizer_service.hpp"

void RunServer(const std::string& server_address) {
    SceneOptimizerService service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "SceneOptimizer agent listening on " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    std::string server_address = "0.0.0.0:50051";

    if (argc > 1) {
        server_address = argv[1];
    }

    std::cout << "Starting SceneOptimizer Agent..." << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;

    RunServer(server_address);

    return 0;
}
```

#### optimizer_service.cpp - Service Implementation

```cpp
// agents/scene-optimizer-agent/src/optimizer_service.cpp
#include "optimizer_service.hpp"
#include <iostream>
#include <fstream>

grpc::Status SceneOptimizerService::OptimizeScene(
    grpc::ServerContext* context,
    const OptimizeRequest* request,
    OptimizeResponse* response) {

    std::cout << "Optimizing scene: " << request->scene_path() << std::endl;

    try {
        // Load scene (using tinygltf or similar - NOT engine code!)
        // ... scene loading logic ...

        // Perform optimization
        // ... optimization logic ...

        // Return results
        response->set_success(true);
        response->set_optimized_path("/tmp/optimized_scene.gltf");
        response->mutable_stats()->set_original_vertices(10000);
        response->mutable_stats()->set_optimized_vertices(5000);
        response->mutable_stats()->set_reduction_percentage(50.0f);

        return grpc::Status::OK;

    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_error_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status SceneOptimizerService::Healthcheck(
    grpc::ServerContext* context,
    const Empty* request,
    HealthStatus* response) {

    response->set_healthy(true);
    response->set_version("1.0.0");
    return grpc::Status::OK;
}
```

### 4. Containerize the Agent

```dockerfile
# agents/scene-optimizer-agent/Dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libgrpc++1.51 \
    libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*

COPY scene-optimizer-agent /usr/local/bin/

EXPOSE 50051

CMD ["scene-optimizer-agent", "0.0.0.0:50051"]
```

```yaml
# docker-compose.yml
version: '3.8'

services:
  scene-optimizer:
    build: ./agents/scene-optimizer-agent
    ports:
      - "50051:50051"
    restart: unless-stopped

  compute-agent:
    build: ./agents/compute-agent
    ports:
      - "50052:50052"
    restart: unless-stopped
```

### 5. Client Integration (From Engine or Any App)

Applications connect to agents as external services:

```cpp
// In your Qimmiit application (or any other app!)
#include <grpcpp/grpcpp.h>
#include "optimizer.grpc.pb.h"

class OptimizerClient {
public:
    OptimizerClient(const std::string& agent_address) {
        auto channel = grpc::CreateChannel(
            agent_address,
            grpc::InsecureChannelCredentials()
        );
        stub_ = cryo::agents::optimizer::SceneOptimizer::NewStub(channel);
    }

    bool optimizeScene(const std::string& scene_path) {
        cryo::agents::optimizer::OptimizeRequest request;
        request.set_scene_path(scene_path);
        request.mutable_settings()->set_lod_levels(3);

        cryo::agents::optimizer::OptimizeResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->OptimizeScene(&context, request, &response);

        if (status.ok() && response.success()) {
            std::cout << "Optimized: " << response.optimized_path() << std::endl;
            std::cout << "Reduction: " << response.stats().reduction_percentage() << "%" << std::endl;
            return true;
        }

        std::cerr << "Failed: " << response.error_message() << std::endl;
        return false;
    }

private:
    std::unique_ptr<cryo::agents::optimizer::SceneOptimizer::Stub> stub_;
};

// Usage
int main() {
    // Connect to agent (could be local or remote)
    OptimizerClient optimizer("localhost:50051");

    optimizer.optimizeScene("assets/scene.gltf");

    return 0;
}
```

## Agent Lifecycle Management

### Starting Agents

#### Manual Start
```bash
# Start agent directly
cd agents/scene-optimizer-agent/build
./scene-optimizer-agent 0.0.0.0:50051
```

#### Docker Compose
```bash
# Start all agents
docker-compose up -d

# View logs
docker-compose logs -f scene-optimizer

# Stop all agents
docker-compose down
```

#### Systemd Service
```ini
# /etc/systemd/system/scene-optimizer.service
[Unit]
Description=Scene Optimizer Agent
After=network.target

[Service]
Type=simple
User=qimmiit
ExecStart=/usr/local/bin/scene-optimizer-agent 0.0.0.0:50051
Restart=always

[Install]
WantedBy=multi-user.target
```

### Service Discovery

For dynamic environments:

```cpp
// Simple service registry
class AgentRegistry {
public:
    void registerAgent(const std::string& name, const std::string& endpoint) {
        agents_[name] = endpoint;
    }

    std::string getAgentEndpoint(const std::string& name) {
        auto it = agents_.find(name);
        return it != agents_.end() ? it->second : "";
    }

private:
    std::map<std::string, std::string> agents_;
};

// Usage
AgentRegistry registry;
registry.registerAgent("scene-optimizer", "localhost:50051");
registry.registerAgent("compute", "compute.example.com:50052");

auto endpoint = registry.getAgentEndpoint("scene-optimizer");
OptimizerClient client(endpoint);
```

## Multi-Language Agents

Since agents are independent, you can use **any language**:

### Python Agent Example

```python
# agents/data-processor-agent/server.py
from concurrent import futures
import grpc
import processor_pb2
import processor_pb2_grpc

class DataProcessorService(processor_pb2_grpc.DataProcessorServicer):
    def ProcessData(self, request, context):
        print(f"Processing data: {request.data_id}")

        # Do processing (completely independent from Qimmiit engine)
        result = self.do_processing(request.input_data)

        return processor_pb2.ProcessResponse(
            success=True,
            output_data=result
        )

    def do_processing(self, data):
        # Your Python logic here
        return b"processed data"

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    processor_pb2_grpc.add_DataProcessorServicer_to_server(
        DataProcessorService(), server
    )
    server.add_insecure_port('[::]:50053')
    print("DataProcessor agent listening on port 50053")
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
```

### Go Agent Example

```go
// agents/analytics-agent/main.go
package main

import (
    "context"
    "log"
    "net"
    "google.golang.org/grpc"
    pb "analytics-agent/proto"
)

type server struct {
    pb.UnimplementedAnalyticsAgentServer
}

func (s *server) AnalyzeData(ctx context.Context, req *pb.AnalyzeRequest) (*pb.AnalyzeResponse, error) {
    log.Printf("Analyzing: %s", req.DataPath)

    // Do analysis (completely independent from Qimmiit)
    // ...

    return &pb.AnalyzeResponse{
        Success: true,
        Results: "analysis complete",
    }, nil
}

func main() {
    lis, err := net.Listen("tcp", ":50054")
    if err != nil {
        log.Fatalf("failed to listen: %v", err)
    }

    s := grpc.NewServer()
    pb.RegisterAnalyticsAgentServer(s, &server{})

    log.Println("Analytics agent listening on port 50054")
    if err := s.Serve(lis); err != nil {
        log.Fatalf("failed to serve: %v", err)
    }
}
```

## Best Practices

### 1. Zero Engine Dependencies

**NEVER link against engine libraries:**

```cmake
# ❌ WRONG - Creates dependency on engine
target_link_libraries(my-agent
    nanook-core          # NO!
    arctic-graphics      # NO!
)

# ✅ CORRECT - Only external dependencies
target_link_libraries(my-agent
    gRPC::grpc++
    protobuf::libprotobuf
    # Only standard libraries
)
```

### 2. Versioned APIs

```protobuf
// Use versioned package names
package cryo.agents.optimizer.v1;

// Version in messages
message AgentInfo {
    string version = 1;  // e.g., "1.2.3"
    int32 api_version = 2;  // e.g., 1
}
```

### 3. Health Checks

Always implement health checks:

```protobuf
service MyAgent {
    rpc Healthcheck(Empty) returns (HealthStatus);
    // ... other methods
}
```

### 4. Graceful Shutdown

```cpp
#include <csignal>
#include <atomic>

std::atomic<bool> shutdown_requested(false);

void signalHandler(int signum) {
    std::cout << "Shutdown requested..." << std::endl;
    shutdown_requested = true;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // ... start server

    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup
    server->Shutdown();
    return 0;
}
```

### 5. Logging & Monitoring

```cpp
// Use structured logging
#include <spdlog/spdlog.h>

void logRequest(const Request& req) {
    spdlog::info("agent=scene-optimizer action=process scene={}",
                 req.scene_path());
}
```

### 6. Configuration

```cpp
// agents/config.cpp
struct AgentConfig {
    std::string host = "0.0.0.0";
    int port = 50051;
    std::string log_level = "info";

    static AgentConfig fromEnv() {
        AgentConfig config;
        if (const char* host = std::getenv("AGENT_HOST")) {
            config.host = host;
        }
        if (const char* port = std::getenv("AGENT_PORT")) {
            config.port = std::stoi(port);
        }
        return config;
    }
};
```

## Example Agents

### 1. Compute Agent (C++)
- Heavy computation tasks
- No UI, pure processing
- High performance requirements

### 2. Studio Server Agent (C++ with HTTP)
- Rendering preview images
- Web-accessible HTTP API
- Stateless rendering

### 3. Asset Pipeline Agent (Python)
- File format conversions
- Batch processing
- Python ecosystem tools

### 4. Analytics Agent (Go)
- Data collection and analysis
- High concurrency
- Cloud integration

## Testing Agents

### Unit Tests
Test business logic independently:

```cpp
TEST(OptimizerTest, OptimizationLogic) {
    // Test the optimization algorithm directly
    Scene scene = loadTestScene();
    Scene optimized = optimizeScene(scene, settings);
    EXPECT_LT(optimized.vertex_count, scene.vertex_count);
}
```

### Integration Tests
Test the gRPC interface:

```cpp
TEST(OptimizerAgentTest, gRPCInterface) {
    // Start agent in test mode
    auto agent = startAgentForTest(50099);

    // Connect client
    auto channel = grpc::CreateChannel("localhost:50099",
                                      grpc::InsecureChannelCredentials());
    auto stub = SceneOptimizer::NewStub(channel);

    // Test request
    OptimizeRequest req;
    req.set_scene_path("test.gltf");

    OptimizeResponse resp;
    grpc::ClientContext ctx;

    grpc::Status status = stub->OptimizeScene(&ctx, req, &resp);
    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(resp.success());

    // Cleanup
    stopAgentForTest(agent);
}
```

## Deployment

### Local Development
```bash
# Build agents
cd agents/scene-optimizer-agent
mkdir build && cd build
cmake ..
make

# Run
./scene-optimizer-agent 0.0.0.0:50051
```

### Production (Docker)
```bash
# Build
docker build -t scene-optimizer-agent agents/scene-optimizer-agent/

# Run
docker run -d -p 50051:50051 --name optimizer scene-optimizer-agent

# Logs
docker logs -f optimizer
```

### Kubernetes
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: scene-optimizer
spec:
  replicas: 3
  selector:
    matchLabels:
      app: scene-optimizer
  template:
    metadata:
      labels:
        app: scene-optimizer
    spec:
      containers:
      - name: optimizer
        image: scene-optimizer-agent:1.0.0
        ports:
        - containerPort: 50051
        env:
        - name: AGENT_PORT
          value: "50051"
---
apiVersion: v1
kind: Service
metadata:
  name: scene-optimizer
spec:
  selector:
    app: scene-optimizer
  ports:
  - port: 50051
    targetPort: 50051
```

## Conclusion

Agents in the Qimmiit ecosystem are **completely independent programs** that provide specialized services. By keeping agents separate from the core engine, you achieve:

- **True Independence**: No engine dependencies whatsoever
- **Language Freedom**: Use the best language for each agent
- **Scalability**: Scale agents independently
- **Reliability**: Agent crashes don't affect the engine
- **Flexibility**: Deploy, update, version independently

### Key Takeaways

1. **Agents are independent executables, not modules**
2. **Zero dependencies on engine code**
3. **Communication only through gRPC/HTTP**
4. **Can be written in any language**
5. **Deployed and scaled independently**

### Further Reading

- [gRPC Documentation](https://grpc.io/docs/)
- [Protocol Buffers Guide](https://protobuf.dev/)
- [Docker Compose](https://docs.docker.com/compose/)
- [Kubernetes](https://kubernetes.io/docs/)

---

*"Agents are independent scouts that operate autonomously, reporting back through well-defined channels."*
