# Umiak Agent

**Umiak is an agent** - a gRPC service for high-level scene manipulation.

## What is Umiak?

An independent gRPC service that understands scene semantics:
- Find entities by name: "suzanne-in-box"
- Move, rotate, scale entities
- Analyze scene structure
- Batch operations

## Running Umiak

```bash
# Start the agent
./umiak-agent --port 50060 --scene /path/to/scene.gltf
```

## Using Umiak

```cpp
// Connect to Umiak via gRPC
auto channel = grpc::CreateChannel("localhost:50060",
                                  grpc::InsecureChannelCredentials());
auto umiak = UmiakAgent::NewStub(channel);

// Find "suzanne-in-box"
EntityQuery query;
query.set_name_pattern("suzanne-in-box");

EntityResult result;
grpc::ClientContext ctx;
umiak->FindEntity(&ctx, query, &result);

// Move to [2, 2, 2]
MoveEntityRequest moveReq;
moveReq.mutable_entity()->set_node_id(result.entity().node_id());
moveReq.mutable_position()->set_x(2.0f);
moveReq.mutable_position()->set_y(2.0f);
moveReq.mutable_position()->set_z(2.0f);

OperationResult moveResult;
grpc::ClientContext moveCtx;
umiak->MoveEntity(&moveCtx, moveReq, &moveResult);
```

## Architecture

```
agents/
└── umiak-agent/          # gRPC service for scene manipulation
    ├── proto/
    │   └── umiak.proto
    ├── src/
    │   └── main.cpp
    └── README.md

orchestrators/
├── nanook/               # Full application with window/startup
└── [other orchestrators] # Other complete applications
```

**Orchestrators** = Complete applications (window, startup, event loop)
**Agents** = Services (gRPC, no window, just processing)

---

*"Umiak is an agent, not an orchestrator."*
