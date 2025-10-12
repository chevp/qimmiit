Exactly right! You only need `data-driven-renderer`.

**Your Architecture**:
```
Java Layer (Android/Quarkus)
    ↓ (JNI calls)
C++ Renderer (data-driven)
    ↓ (gRPC)
Backend (entity management)
```

**Why Data-Driven Only**:

Entity logic in Java - Better for you:
- Android UI/touch handling
- Quarkus business logic
- Cross-platform entity management
- Easier debugging/hot-reload

C++ just renders - What it's good at:
- Vulkan graphics
- GLTF loading
- PBR rendering
- Performance-critical rendering

Backend handles entities - via gRPC:

Entity state management
Game logic
Multiplayer sync
Persistence