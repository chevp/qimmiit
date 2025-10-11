# Data-Driven Renderer

A C++ gRPC client renderer that receives real-time shader updates from a backend server for dynamic visual control.

## Architecture

```
Backend Server (Quarkus/Spring Boot)
    ↓ gRPC bidirectional streaming
C++ Data-Driven Renderer
    ├── DataDrivenConfigParser (loads data-driven.index.xml)
    ├── DataDrivenShaderManager (manages shader instances)
    ├── GrpcRendererClient (receives shader updates)
    └── Vulkan Rendering (TODO: integrate with coregfx)
```

## Key Concept

**Server-Driven Rendering** - Backend server controls shader parameters in real-time without client redeployment.

- **Backend Server**: Sends shader updates, scene changes, UI style updates via gRPC
- **C++ Renderer**: Receives updates and applies them to shader instances
- **data-driven.index.xml**: Defines initial shader templates and instances

## Use Cases

- **Server-Driven UI**: Change button materials remotely without client updates
- **Dynamic VFX**: Update smoke/fire/particle parameters in real-time
- **A/B Testing**: Test different material variations server-side
- **Live Events**: Change environment/lighting for special events
- **Lightweight Updates**: Send only changed parameters over network

## Configuration

### data-driven.index.xml

Defines shader templates, instances, camera, and backend connection:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<DataDrivenScene name="Example Scene" version="1.0">
    <ShaderTemplates>
        <ShaderTemplate id="pbr_standard">
            <VertexShader>shaders/pbr.vert.spv</VertexShader>
            <FragmentShader>shaders/pbr.frag.spv</FragmentShader>
        </ShaderTemplate>
    </ShaderTemplates>

    <ShaderInstances>
        <ShaderInstance id="metal_crate_01" template="pbr_standard">
            <Uniforms>
                <Uniform name="metallic_factor" type="float" value="1.0"/>
                <Uniform name="roughness_factor" type="float" value="0.5"/>
            </Uniforms>
            <Textures>
                <Texture name="base_color_texture" path="textures/metal_plate.png"/>
            </Textures>
        </ShaderInstance>
    </ShaderInstances>

    <Camera>
        <Position x="0" y="3" z="15"/>
        <FOV>60.0</FOV>
    </Camera>

    <Backend>
        <Host>localhost</Host>
        <Port>50051</Port>
        <Protocol>grpc</Protocol>
    </Backend>
</DataDrivenScene>
```

## gRPC Protocol

Based on [shader_updates.proto](shader_updates.proto):

### Server → Client (Shader Updates)

```protobuf
message ShaderUpdate {
  string shader_template_id = 1;
  string instance_id = 2;
  map<string, UniformValue> uniforms = 3;
  map<string, string> textures = 4;
}
```

### Client → Server (Telemetry)

```protobuf
message FrameRendered {
  uint64 frame_number = 1;
  float frame_time_ms = 2;
}
```

## Usage

### Build

```bash
cmake --build build-x86 --config Debug --target data_driven_coregfx_renderer
```

### Run

```bash
# With gRPC enabled (default)
./build-x86/bin/Debug/data-driven-coregfx-renderer.exe data-driven.index.xml

# Offline mode (no gRPC)
./build-x86/bin/Debug/data-driven-coregfx-renderer.exe --no-grpc data-driven.index.xml

# Headless mode
./build-x86/bin/Debug/data-driven-coregfx-renderer.exe --headless data-driven.index.xml

# Custom config file
./build-x86/bin/Debug/data-driven-coregfx-renderer.exe --config custom-scene.xml
```

### Command Line Options

```
Usage: data-driven-coregfx-renderer [options] [config.xml]

Options:
  --config <file>     XML configuration file (default: data-driven.index.xml)
  --no-grpc           Disable gRPC client (offline mode)
  --headless          Headless mode (no window)
  --help              Show help message
```

## Implementation Status

### ✅ Completed
- XML configuration parser ([DataDrivenConfigParser.cpp](src/DataDrivenConfigParser.cpp))
- Shader instance manager ([DataDrivenShaderManager.cpp](src/DataDrivenShaderManager.cpp))
- gRPC client with bidirectional streaming ([GrpcRendererClient.cpp](src/GrpcRendererClient.cpp))
- Protobuf definitions ([shader_updates.proto](shader_updates.proto))
- Command line parsing and application structure ([main.cpp](src/main.cpp))

### 🚧 TODO
- Integrate with coregfx Vulkan rendering system
- Window creation and event handling
- Actual shader compilation and pipeline creation
- Apply shader updates to Vulkan descriptors
- Implement scene reloading on SceneUpdate messages
- UI style updates for ImGui widgets

## Architecture Details

### DataDrivenShaderManager
Manages shader templates and instances:

```cpp
// Load shader templates from XML
shaderManager.loadTemplates(config.shaderTemplates);

// Initialize instances
shaderManager.initializeInstances(config.shaderInstances);

// Update instance from gRPC
shaderManager.updateShaderInstance(shaderUpdate);
```

### GrpcRendererClient
Bidirectional streaming gRPC client:

```cpp
// Create client
GrpcRendererClient grpcClient(backendConfig);

// Set callbacks
grpcClient.setShaderUpdateCallback([&](const rendering::ShaderUpdate& update) {
    shaderManager.updateShaderInstance(update);
});

// Connect and start streaming
grpcClient.connect();
```

## Related Documentation

- [DATA_DRIVEN_API_SPEC.md](docs/DATA_DRIVEN_API_SPEC.md) - Complete API specification
- [SCENE_FORMATS.md](docs/SCENE_FORMATS.md) - XML and protobuf format details
- [SUMMARY.md](docs/SUMMARY.md) - Architecture summary
- [INFO.md](docs/INFO.md) - Additional context

## Comparison with elyrion.coregfx.renderer

| Feature | elyrion.coregfx.renderer | data-driven-renderer |
|---------|--------------------------|----------------------|
| Configuration | Elyrion XML (entities) | data-driven.index.xml (shaders) |
| Updates | Static scene files | Dynamic gRPC updates |
| Network | HTTP REST API | gRPC bidirectional streaming |
| Focus | Full scene rendering | Shader instance control |
| Use Case | Desktop testing/validation | Server-driven visuals |

## Example Workflow

1. **Start renderer**: Load initial scene from `data-driven.index.xml`
2. **Connect to backend**: Establish gRPC streaming connection
3. **Receive updates**: Backend sends shader parameter changes
4. **Apply updates**: ShaderManager updates Vulkan descriptors
5. **Render frame**: Apply updated shader parameters to geometry
6. **Send telemetry**: Report frame time and events back to server

---

**Status**: 📋 **Implementation Phase** - Core architecture complete, Vulkan integration pending
