# coregfx Library Examples

This directory contains example programs demonstrating the usage of the coregfx library.

## Example Program: main.cpp

Demonstrates the following coregfx features:

### 1. Thread Pool (`coregfx::ThreadPool`)
- Creating a thread pool with worker threads
- Enqueuing asynchronous tasks
- Collecting results via futures
- **Independent**: No Vulkan or gRPC dependencies

### 2. Resource Manager (`ResourceManager`)
- Singleton pattern for resource management
- File loading and caching
- Fast repeated access to cached resources
- **Independent**: No Vulkan or gRPC dependencies

### 3. GLTF Loader (`GltfLoader`)
- Loading GLTF 2.0 model files
- Caching loaded models by ID
- Integration with TinyGLTF library
- **Independent**: Only requires TinyGLTF (no Vulkan/gRPC)

### 4. Elyrion XML Parser (`xmlelyrion::ElyrionLoader`)
- Parsing Elyrion scene definition XML files
- Converting to protobuf models (nyx::Elyrion)
- Asset and scene management
- **Note**: Depends on protobuf (uses gRPC protobuf definitions)

## Building the Example

### As part of coregfx project:

```bash
# From arctic-game-client root directory
cmake --build build-x86 --config Debug --target coregfx_example
```

### Standalone build:

```bash
cd coregfx/examples
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

## Running the Example

```bash
# Windows (x86 Debug build)
./build-x86/bin/Debug/coregfx_example.exe

# Or from build directory
./bin/Debug/coregfx_example.exe
```

## Expected Output

The example will demonstrate:
1. Concurrent task execution using thread pool
2. File resource loading and caching
3. GLTF loader API usage
4. Elyrion XML parsing with complete scene details

## Code Structure

```cpp
// Thread Pool Usage
coregfx::ThreadPool pool(4);
auto result = pool.enqueue([](){ return 42; });

// Resource Manager
auto& rm = ResourceManager::getInstance();
std::string content = rm.load("file.txt");

// GLTF Loader
auto& loader = GltfLoader::getInstance();
tinygltf::Model model = loader.loadGltfRsc(id, "model.gltf");

// Elyrion XML Parser
nyx::Elyrion scene;
xmlelyrion::ElyrionLoader::loadXmlFromFile("scene.xml", scene);
```

## Dependencies

- **C++ Standard Library** (STL)
- **TinyGLTF** (for GLTF loading)
- **tinyxml2** (for XML parsing)
- **Protobuf** (for Elyrion scene models)
- **coregfx** (main library)

## Notes

- Thread pool and resource manager are **fully independent** utilities
- GLTF loader only depends on TinyGLTF (no Vulkan)
- Elyrion XML parser uses protobuf but not Vulkan
- For full Vulkan rendering, see `elyrion.coregfx.renderer` project
