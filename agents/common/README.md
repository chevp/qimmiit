# Agent Common Protobuf Definitions

## What's Here

**Only protobuf definitions** for optional standard health checks.

```
agents/common/
├── proto/
│   └── agent_common.proto    # Optional health check service
└── CMakeLists.txt
```

## Philosophy

**Agents are independent programs**. They communicate via gRPC/HTTP only.

Agents can be written in **any language** (C++, Python, Go, Rust, etc.).

The only thing agents can share is: **protobuf message definitions**.

## agent_common.proto

Optional standard health check service:

```protobuf
service AgentHealth {
    rpc Check(Empty) returns (HealthCheckResponse);
    rpc GetMetadata(Empty) returns (AgentMetadata);
    rpc GetMetrics(Empty) returns (MetricsResponse);
}
```

**This is completely optional!** Agents don't have to use it.

## Usage

### For Monitoring Tools

If agents implement the common health service, you can check them uniformly:

```bash
grpcurl -plaintext localhost:50060 cryo.agents.common.AgentHealth/Check
grpcurl -plaintext localhost:50061 cryo.agents.common.AgentHealth/Check
grpcurl -plaintext localhost:50062 cryo.agents.common.AgentHealth/Check
```

### In Your Agent (C++)

```cpp
// Optional: Implement the health service
#include <agent_common.grpc.pb.h>

class MyHealthService : public cryo::agents::common::AgentHealth::Service {
    grpc::Status Check(/*...*/) override {
        // Return health status
    }
};
```

### In Your Agent (Python)

```python
import agent_common_pb2
import agent_common_pb2_grpc

class MyHealthService(agent_common_pb2_grpc.AgentHealthServicer):
    def Check(self, request, context):
        return agent_common_pb2.HealthCheckResponse(
            status=agent_common_pb2.HEALTHY,
            message="Running"
        )
```

### Or Don't Use It

Your agent can define everything itself:

```protobuf
// my_agent.proto
service MyAgent {
    rpc DoWork(Request) returns (Response);
    rpc MyHealthCheck(Empty) returns (MyHealthResponse);  // Your own
}
```

## Building

### For C++ Agents

```bash
cd agents/common
mkdir build && cd build
cmake ..
make
```

Then link against the generated code:
```cmake
target_link_libraries(my-agent PRIVATE agent-common-proto gRPC::grpc++)
```

### For Other Languages

```bash
# Python
python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. proto/agent_common.proto

# Go
protoc --go_out=. --go-grpc_out=. proto/agent_common.proto

# Rust
protoc --rust_out=. proto/agent_common.proto
```

---

*"The only thing agents share is the language they speak: Protocol Buffers"*
