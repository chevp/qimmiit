# Cryo Engine CLI Specification

**Automatic Specification Generation System**

---

## Overview

The `cryo-engine-cli.exe spec` command provides **automatic introspection** of the entire Cryo Engine architecture. It generates a comprehensive specification by querying all registered modules, their capabilities, configuration options, events, and APIs at runtime.

---

## Usage

```bash
# Full specification (all modules, events, config)
cryo-engine-cli.exe spec

# Specific module specification
cryo-engine-cli.exe spec --module RendererModule

# Output formats
cryo-engine-cli.exe spec --format markdown    # Default
cryo-engine-cli.exe spec --format json        # Machine-readable
cryo-engine-cli.exe spec --format yaml        # Human-readable structured
cryo-engine-cli.exe spec --format html        # Web documentation

# Save to file
cryo-engine-cli.exe spec --output spec.md
cryo-engine-cli.exe spec --format json --output spec.json

# Specific sections
cryo-engine-cli.exe spec --section modules
cryo-engine-cli.exe spec --section events
cryo-engine-cli.exe spec --section config
cryo-engine-cli.exe spec --section api
cryo-engine-cli.exe spec --section all        # Default
```

---

## Generated Specification Structure

### Full Specification Output

```
┌─────────────────────────────────────────────────────────┐
│              CRYO ENGINE SPECIFICATION                  │
│              Generated: 2025-10-10 14:32:15             │
│              Version: 1.0.0                             │
└─────────────────────────────────────────────────────────┘

1. ENGINE INFORMATION
   - Version
   - Build configuration
   - Platform
   - Runtime statistics

2. REGISTERED MODULES
   For each module:
   - Name, type, version
   - Description
   - Dependencies
   - Public API
   - Configuration options
   - Events published/subscribed
   - Runtime statistics

3. EVENT BUS
   - Event types catalog
   - Publishers and subscribers
   - Event flow diagram
   - Message format specifications

4. CONFIGURATION
   - All configuration sections
   - Available options per section
   - Data types and default values
   - Validation rules
   - Current loaded values

5. TASK SCHEDULER
   - Task queue statistics
   - Priority levels
   - Active/pending tasks

6. MEMORY MANAGER
   - Memory pool configuration
   - Allocation statistics
   - Peak usage

7. API ENDPOINTS (if HTTP module enabled)
   - REST API routes
   - WebSocket endpoints
   - Request/response formats

8. COMMAND LINE OPTIONS
   - All available flags
   - Environment variables
   - Configuration file paths
```

---

## Implementation Design

### IModule Extensions for Introspection

Every module must implement these methods for spec generation:

```cpp
// IModule.hpp - Extended interface

class IModule {
public:
    // ... existing methods ...

    /**
     * Get module specification (for automatic documentation)
     */
    virtual ModuleSpec getSpecification() const = 0;
};

struct ModuleSpec {
    // Basic info
    std::string name;
    std::string version;
    ModuleType type;
    std::string description;

    // Dependencies
    std::vector<std::string> dependencies;

    // API
    std::vector<APIMethod> publicMethods;

    // Configuration
    std::vector<ConfigOption> configOptions;

    // Events
    std::vector<EventType> publishedEvents;
    std::vector<EventType> subscribedEvents;

    // Runtime stats (optional)
    std::map<std::string, std::string> statistics;
};

struct APIMethod {
    std::string name;
    std::string description;
    std::vector<Parameter> parameters;
    std::string returnType;
    std::string example;
};

struct Parameter {
    std::string name;
    std::string type;
    std::string description;
    bool required;
    std::string defaultValue;
};

struct ConfigOption {
    std::string key;
    std::string type;
    std::string description;
    std::string defaultValue;
    std::string validationRule;
    std::string currentValue;
};
```

### Example Module Specification

```cpp
// RendererModule.cpp

ModuleSpec RendererModule::getSpecification() const {
    ModuleSpec spec;

    spec.name = "RendererModule";
    spec.version = "1.0.0";
    spec.type = ModuleType::RENDERER;
    spec.description = "Vulkan-based PBR rendering engine with GLTF support";

    // Dependencies
    spec.dependencies = {"CacheModule", "AssetModule"};

    // Public API
    spec.publicMethods = {
        {
            .name = "loadScene",
            .description = "Load a scene from file or cache",
            .parameters = {
                {
                    .name = "sceneFile",
                    .type = "std::string",
                    .description = "Path to scene file (.arctic format)",
                    .required = true,
                    .defaultValue = ""
                }
            },
            .returnType = "bool",
            .example = "renderer->loadScene(\"scenes/tutorial_01.arctic\")"
        },
        {
            .name = "setCameraPosition",
            .description = "Set camera position in world space",
            .parameters = {
                {
                    .name = "position",
                    .type = "glm::vec3",
                    .description = "Camera position (x, y, z)",
                    .required = true,
                    .defaultValue = "(0, 0, 0)"
                }
            },
            .returnType = "void",
            .example = "renderer->setCameraPosition(glm::vec3(0, 3, 15))"
        },
        {
            .name = "takeScreenshot",
            .description = "Capture framebuffer to PNG file",
            .parameters = {
                {
                    .name = "filename",
                    .type = "std::string",
                    .description = "Output PNG filename",
                    .required = true,
                    .defaultValue = "screenshot.png"
                }
            },
            .returnType = "bool",
            .example = "renderer->takeScreenshot(\"output.png\")"
        }
    };

    // Configuration options
    spec.configOptions = {
        {
            .key = "renderer.vulkan-validation",
            .type = "bool",
            .description = "Enable Vulkan validation layers (debug builds)",
            .defaultValue = "false",
            .validationRule = "true|false",
            .currentValue = vulkanValidationEnabled_ ? "true" : "false"
        },
        {
            .key = "renderer.vsync",
            .type = "bool",
            .description = "Enable vertical synchronization",
            .defaultValue = "true",
            .validationRule = "true|false",
            .currentValue = vsyncEnabled_ ? "true" : "false"
        },
        {
            .key = "renderer.msaa-samples",
            .type = "int",
            .description = "MSAA sample count (1, 2, 4, 8)",
            .defaultValue = "1",
            .validationRule = "1|2|4|8",
            .currentValue = std::to_string(msaaSamples_)
        },
        {
            .key = "renderer.default-width",
            .type = "int",
            .description = "Default window width (pixels)",
            .defaultValue = "1920",
            .validationRule = "range(640, 7680)",
            .currentValue = std::to_string(windowWidth_)
        },
        {
            .key = "renderer.default-height",
            .type = "int",
            .description = "Default window height (pixels)",
            .defaultValue = "1080",
            .validationRule = "range(480, 4320)",
            .currentValue = std::to_string(windowHeight_)
        }
    };

    // Events
    spec.publishedEvents = {
        EventType::SCENE_LOADED,
        EventType::FRAME_RENDERED
    };

    spec.subscribedEvents = {
        EventType::ASSET_READY,
        EventType::CACHE_HIT
    };

    // Runtime statistics
    spec.statistics = {
        {"frames_rendered", std::to_string(framesRendered_)},
        {"avg_frame_time_ms", std::to_string(avgFrameTimeMs_)},
        {"gpu_memory_used_mb", std::to_string(gpuMemoryUsedMB_)},
        {"loaded_entities", std::to_string(loadedEntities_)},
        {"active_textures", std::to_string(activeTextures_)}
    };

    return spec;
}
```

### CentralAgent Spec Generator

```cpp
// CentralAgent.cpp

void CentralAgent::generateSpecification(const std::string& format,
                                         const std::string& outputFile) {
    SpecificationGenerator generator;

    // 1. Engine information
    EngineInfo engineInfo = getEngineInfo();
    generator.addSection("Engine Information", engineInfo);

    // 2. Registered modules
    for (const auto& [name, module] : modules_) {
        ModuleSpec spec = module->getSpecification();
        generator.addModule(spec);
    }

    // 3. Event bus
    EventBusSpec eventBusSpec = eventBus_.getSpecification();
    generator.addSection("Event Bus", eventBusSpec);

    // 4. Configuration
    ConfigSpec configSpec = configManager_.getSpecification();
    generator.addSection("Configuration", configSpec);

    // 5. Task scheduler
    TaskSchedulerSpec taskSpec = taskScheduler_.getSpecification();
    generator.addSection("Task Scheduler", taskSpec);

    // 6. Memory manager
    MemoryManagerSpec memSpec = memoryManager_.getSpecification();
    generator.addSection("Memory Manager", memSpec);

    // 7. Generate output
    if (format == "markdown") {
        std::string markdown = generator.generateMarkdown();
        writeToFile(outputFile.empty() ? "cryo-engine-spec.md" : outputFile, markdown);
    } else if (format == "json") {
        std::string json = generator.generateJSON();
        writeToFile(outputFile.empty() ? "cryo-engine-spec.json" : outputFile, json);
    } else if (format == "yaml") {
        std::string yaml = generator.generateYAML();
        writeToFile(outputFile.empty() ? "cryo-engine-spec.yaml" : outputFile, yaml);
    } else if (format == "html") {
        std::string html = generator.generateHTML();
        writeToFile(outputFile.empty() ? "cryo-engine-spec.html" : outputFile, html);
    }
}

EngineInfo CentralAgent::getEngineInfo() const {
    EngineInfo info;
    info.version = CRYO_ENGINE_VERSION;
    info.buildDate = __DATE__;
    info.buildTime = __TIME__;
    info.buildConfig = CMAKE_BUILD_TYPE;
    info.platform = getPlatformName();
    info.compiler = getCompilerInfo();
    info.moduleCount = modules_.size();
    info.running = running_;
    info.uptime = getUptimeSeconds();
    return info;
}
```

---

## Example Output: Markdown Format

### Full Spec Output

````markdown
# Cryo Engine Specification

**Generated**: 2025-10-10 14:32:15
**Version**: 1.0.0
**Build**: Release x64
**Platform**: Windows 11 (10.0.22621)

---

## 1. Engine Information

| Property | Value |
|----------|-------|
| Version | 1.0.0 |
| Build Date | Oct 10 2025 |
| Build Time | 14:30:22 |
| Build Config | Release |
| Platform | Windows x64 |
| Compiler | MSVC 19.38 |
| Module Count | 5 |
| Running | true |
| Uptime | 3600 seconds |

---

## 2. Registered Modules

### 2.1 RendererModule

**Type**: RENDERER
**Version**: 1.0.0
**Description**: Vulkan-based PBR rendering engine with GLTF support

**Dependencies**:
- CacheModule
- AssetModule

#### Public API

##### `loadScene(sceneFile: string) -> bool`
Load a scene from file or cache

**Parameters**:
- `sceneFile` (string, required): Path to scene file (.arctic format)

**Example**:
```cpp
renderer->loadScene("scenes/tutorial_01.arctic")
```

##### `setCameraPosition(position: vec3) -> void`
Set camera position in world space

**Parameters**:
- `position` (glm::vec3, required): Camera position (x, y, z)
  - Default: `(0, 0, 0)`

**Example**:
```cpp
renderer->setCameraPosition(glm::vec3(0, 3, 15))
```

##### `takeScreenshot(filename: string) -> bool`
Capture framebuffer to PNG file

**Parameters**:
- `filename` (string, required): Output PNG filename
  - Default: `screenshot.png`

**Example**:
```cpp
renderer->takeScreenshot("output.png")
```

#### Configuration Options

| Key | Type | Description | Default | Current | Validation |
|-----|------|-------------|---------|---------|------------|
| `renderer.vulkan-validation` | bool | Enable Vulkan validation layers | false | false | true\|false |
| `renderer.vsync` | bool | Enable vertical synchronization | true | true | true\|false |
| `renderer.msaa-samples` | int | MSAA sample count | 1 | 4 | 1\|2\|4\|8 |
| `renderer.default-width` | int | Default window width (pixels) | 1920 | 1920 | range(640, 7680) |
| `renderer.default-height` | int | Default window height (pixels) | 1080 | 1080 | range(480, 4320) |

#### Events

**Published**:
- `SCENE_LOADED`
- `FRAME_RENDERED`

**Subscribed**:
- `ASSET_READY`
- `CACHE_HIT`

#### Runtime Statistics

| Metric | Value |
|--------|-------|
| frames_rendered | 216000 |
| avg_frame_time_ms | 16.7 |
| gpu_memory_used_mb | 1024 |
| loaded_entities | 42 |
| active_textures | 128 |

---

### 2.2 CacheModule

**Type**: CACHE
**Version**: 1.0.0
**Description**: LRU/TTL caching system with SQLite persistence

**Dependencies**: (none)

#### Public API

##### `hasScene(sceneId: string) -> bool`
Check if scene exists in cache

**Parameters**:
- `sceneId` (string, required): Scene identifier

**Example**:
```cpp
if (cache->hasScene("tutorial_01")) { /* ... */ }
```

##### `loadScene(sceneId: string) -> Scene`
Load scene from cache

**Parameters**:
- `sceneId` (string, required): Scene identifier

**Example**:
```cpp
auto scene = cache->loadScene("tutorial_01")
```

##### `saveScene(scene: Scene) -> void`
Save scene to cache

**Parameters**:
- `scene` (Scene, required): Scene data to cache

**Example**:
```cpp
cache->saveScene(myScene)
```

#### Configuration Options

| Key | Type | Description | Default | Current | Validation |
|-----|------|-------------|---------|---------|------------|
| `cache.enabled` | bool | Enable caching system | true | true | true\|false |
| `cache.max-size-gb` | int | Maximum cache size (GB) | 10 | 10 | range(1, 100) |
| `cache.ttl-days` | int | Time-to-live (days) | 7 | 7 | range(1, 365) |
| `cache.strategy` | string | Eviction strategy | LRU | LRU | LRU\|LFU\|FIFO |
| `cache.cache-dir` | string | Cache directory path | cache/ | cache/ | path |

#### Events

**Published**:
- `CACHE_HIT`
- `CACHE_MISS`

**Subscribed**:
- `SCENE_LOADED`

#### Runtime Statistics

| Metric | Value |
|--------|-------|
| total_size_mb | 2048 |
| scene_count | 15 |
| asset_count | 342 |
| cache_hit_rate | 0.87 |
| evicted_items | 3 |

---

### 2.3 AssetModule

**Type**: ASSET
**Version**: 1.0.0
**Description**: Asset processing and validation pipeline

**Dependencies**:
- CacheModule

#### Public API

##### `processAsset(assetId: string) -> void`
Process and optimize an asset

**Parameters**:
- `assetId` (string, required): Asset identifier

**Example**:
```cpp
asset->processAsset("models/pilot.glb")
```

##### `validateAsset(assetId: string) -> ValidationResult`
Validate asset integrity

**Parameters**:
- `assetId` (string, required): Asset identifier

**Example**:
```cpp
auto result = asset->validateAsset("textures/diffuse.png")
if (result.valid) { /* ... */ }
```

#### Configuration Options

| Key | Type | Description | Default | Current | Validation |
|-----|------|-------------|---------|---------|------------|
| `asset.asset-root` | string | Asset root directory | assets/vkpbr5 | assets/vkpbr5 | path |
| `asset.enable-compression` | bool | Enable texture compression | true | true | true\|false |
| `asset.texture-format` | string | Target texture format | DDS | DDS | DDS\|KTX2\|PNG |

#### Events

**Published**:
- `ASSET_READY`
- `ASSET_FAILED`

**Subscribed**:
- `CACHE_MISS`

#### Runtime Statistics

| Metric | Value |
|--------|-------|
| processed_assets | 342 |
| failed_validations | 2 |
| avg_process_time_ms | 45.3 |

---

### 2.4 HttpModule

**Type**: HTTP
**Version**: 1.0.0
**Description**: REST API server and WebSocket support

**Dependencies**: (none)

#### Public API

##### `start() -> bool`
Start HTTP server

**Example**:
```cpp
http->start()
```

##### `stop() -> void`
Stop HTTP server

**Example**:
```cpp
http->stop()
```

#### Configuration Options

| Key | Type | Description | Default | Current | Validation |
|-----|------|-------------|---------|---------|------------|
| `http-server.enabled` | bool | Enable HTTP server | true | true | true\|false |
| `http-server.host` | string | Bind host | 0.0.0.0 | 0.0.0.0 | ip |
| `http-server.port` | int | Port number | 52009 | 52009 | range(1024, 65535) |
| `http-server.cors` | bool | Enable CORS | true | true | true\|false |
| `http-server.max-connections` | int | Max concurrent connections | 100 | 100 | range(1, 10000) |

#### Events

**Published**:
- `HTTP_REQUEST`
- `HTTP_RESPONSE`

**Subscribed**: (none)

#### Runtime Statistics

| Metric | Value |
|--------|-------|
| requests_handled | 1523 |
| active_connections | 3 |
| avg_response_time_ms | 12.4 |

#### REST API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/status` | Get engine status |
| GET | `/api/modules` | List registered modules |
| GET | `/api/scenes` | List cached scenes |
| GET | `/api/scenes/{id}` | Get scene by ID |
| POST | `/api/scenes` | Upload new scene |
| GET | `/api/screenshot` | Take screenshot |
| POST | `/api/camera/position` | Set camera position |
| POST | `/api/camera/rotation` | Set camera rotation |

---

### 2.5 ComputeModule

**Type**: COMPUTE
**Version**: 1.0.0
**Description**: Background task processing (shader compilation, asset baking)

**Dependencies**: (none)

#### Public API

##### `submitTask(task: Task, priority: Priority) -> TaskId`
Submit background task

**Parameters**:
- `task` (Task, required): Task to execute
- `priority` (Priority, optional): Task priority
  - Default: `NORMAL`

**Example**:
```cpp
auto taskId = compute->submitTask(myTask, Priority::HIGH)
```

##### `cancelTask(taskId: TaskId) -> bool`
Cancel pending task

**Parameters**:
- `taskId` (TaskId, required): Task identifier

**Example**:
```cpp
compute->cancelTask(taskId)
```

#### Configuration Options

| Key | Type | Description | Default | Current | Validation |
|-----|------|-------------|---------|---------|------------|
| `compute.worker-threads` | int | Number of worker threads | 4 | 4 | range(1, 64) |
| `compute.queue-size` | int | Task queue size | 1000 | 1000 | range(10, 100000) |

#### Events

**Published**:
- `TASK_COMPLETED`
- `TASK_FAILED`

**Subscribed**: (none)

#### Runtime Statistics

| Metric | Value |
|--------|-------|
| tasks_completed | 8234 |
| tasks_pending | 12 |
| tasks_failed | 3 |
| avg_task_time_ms | 234.5 |

---

## 3. Event Bus

**Event Types**: 10

| Event Type | Publishers | Subscribers | Description |
|------------|------------|-------------|-------------|
| SCENE_LOADED | RendererModule | CacheModule | Scene finished loading |
| ASSET_READY | AssetModule | RendererModule | Asset processed and ready |
| ASSET_FAILED | AssetModule | RendererModule | Asset processing failed |
| CACHE_HIT | CacheModule | RendererModule, AssetModule | Cache lookup succeeded |
| CACHE_MISS | CacheModule | AssetModule | Cache lookup failed |
| HTTP_REQUEST | HttpModule | (none) | HTTP request received |
| HTTP_RESPONSE | HttpModule | (none) | HTTP response sent |
| FRAME_RENDERED | RendererModule | (none) | Frame rendered to screen |
| TASK_COMPLETED | ComputeModule | (none) | Background task finished |
| TASK_FAILED | ComputeModule | (none) | Background task failed |

**Event Flow Diagram**:
```
AssetModule → ASSET_READY → RendererModule
CacheModule → CACHE_HIT → RendererModule
CacheModule → CACHE_MISS → AssetModule
RendererModule → SCENE_LOADED → CacheModule
```

---

## 4. Configuration

**Sections**: 5
**Total Options**: 20
**Source File**: `arctic.config.xml`

### 4.1 Engine Section

| Key | Type | Default | Current | Description |
|-----|------|---------|---------|-------------|
| `engine.thread-count` | int | 8 | 8 | Worker thread count |
| `engine.log-level` | string | DEBUG | DEBUG | Logging level (DEBUG\|INFO\|WARN\|ERROR) |
| `engine.memory-pool-mb` | int | 512 | 512 | Memory pool size (MB) |

### 4.2 Renderer Section

(See RendererModule configuration above)

### 4.3 Cache Section

(See CacheModule configuration above)

### 4.4 Asset Section

(See AssetModule configuration above)

### 4.5 HTTP Server Section

(See HttpModule configuration above)

### 4.6 Compute Section

(See ComputeModule configuration above)

---

## 5. Task Scheduler

**Worker Threads**: 8
**Pending Tasks**: 12
**Completed Tasks**: 8234
**Failed Tasks**: 3
**Average Task Time**: 234.5 ms

**Priority Queue**:
| Priority | Tasks |
|----------|-------|
| CRITICAL | 0 |
| HIGH | 3 |
| NORMAL | 7 |
| LOW | 2 |

---

## 6. Memory Manager

**Pool Size**: 512 MB
**Allocated**: 287 MB (56%)
**Peak Allocated**: 412 MB (80%)
**Active Allocations**: 1234
**Total Allocations**: 45678
**Total Frees**: 44444

---

## 7. Command Line Options

### Global Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--config <file>` | string | Configuration file path | arctic.config.xml |
| `--log-level <level>` | string | Logging level | DEBUG |
| `--headless` | flag | Run without GUI | false |
| `--help` | flag | Show help message | - |
| `--version` | flag | Show version | - |
| `--spec` | flag | Show specification | - |

### Renderer Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--scene <file>` | string | Scene file to load | (none) |
| `--camera-pos <x,y,z>` | vec3 | Camera position | (0, 3, 15) |
| `--camera-rot <x,y,z>` | vec3 | Camera rotation | (0, 0, 0) |
| `--screenshot <file>` | string | Take screenshot on start | (none) |
| `--width <pixels>` | int | Window width | 1920 |
| `--height <pixels>` | int | Window height | 1080 |

### HTTP Server Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--http-server` | flag | Enable HTTP server | false |
| `--http-port <port>` | int | HTTP port | 52009 |

---

**End of Specification**

Generated by Cryo Engine Specification Generator v1.0.0
````

---

## Example Output: JSON Format

```json
{
  "engine": {
    "version": "1.0.0",
    "buildDate": "Oct 10 2025",
    "buildTime": "14:30:22",
    "buildConfig": "Release",
    "platform": "Windows x64",
    "compiler": "MSVC 19.38",
    "moduleCount": 5,
    "running": true,
    "uptime": 3600
  },
  "modules": [
    {
      "name": "RendererModule",
      "version": "1.0.0",
      "type": "RENDERER",
      "description": "Vulkan-based PBR rendering engine with GLTF support",
      "dependencies": ["CacheModule", "AssetModule"],
      "api": [
        {
          "name": "loadScene",
          "description": "Load a scene from file or cache",
          "parameters": [
            {
              "name": "sceneFile",
              "type": "string",
              "description": "Path to scene file (.arctic format)",
              "required": true,
              "default": ""
            }
          ],
          "returnType": "bool",
          "example": "renderer->loadScene(\"scenes/tutorial_01.arctic\")"
        }
      ],
      "config": [
        {
          "key": "renderer.vulkan-validation",
          "type": "bool",
          "description": "Enable Vulkan validation layers",
          "default": "false",
          "current": "false",
          "validation": "true|false"
        }
      ],
      "events": {
        "published": ["SCENE_LOADED", "FRAME_RENDERED"],
        "subscribed": ["ASSET_READY", "CACHE_HIT"]
      },
      "statistics": {
        "frames_rendered": 216000,
        "avg_frame_time_ms": 16.7,
        "gpu_memory_used_mb": 1024,
        "loaded_entities": 42,
        "active_textures": 128
      }
    }
  ],
  "eventBus": {
    "eventTypes": [
      {
        "name": "SCENE_LOADED",
        "publishers": ["RendererModule"],
        "subscribers": ["CacheModule"],
        "description": "Scene finished loading"
      }
    ]
  },
  "config": {
    "sections": {
      "engine": [
        {
          "key": "thread-count",
          "type": "int",
          "default": 8,
          "current": 8,
          "description": "Worker thread count"
        }
      ]
    }
  },
  "taskScheduler": {
    "workerThreads": 8,
    "pendingTasks": 12,
    "completedTasks": 8234,
    "failedTasks": 3,
    "avgTaskTimeMs": 234.5
  },
  "memoryManager": {
    "poolSizeMB": 512,
    "allocatedMB": 287,
    "peakAllocatedMB": 412,
    "activeAllocations": 1234
  }
}
```

---

## Implementation Files

### File Structure

```
cryo-engine/
├── src/
│   ├── core/
│   │   ├── CentralAgent.hpp
│   │   ├── CentralAgent.cpp
│   │   ├── IModule.hpp                  # Extended with getSpecification()
│   │   └── SpecificationGenerator.hpp   # NEW - Spec generation
│   │   └── SpecificationGenerator.cpp   # NEW
│   │
│   ├── modules/
│   │   ├── RendererModule.cpp           # Implements getSpecification()
│   │   ├── CacheModule.cpp              # Implements getSpecification()
│   │   ├── AssetModule.cpp              # Implements getSpecification()
│   │   ├── HttpModule.cpp               # Implements getSpecification()
│   │   └── ComputeModule.cpp            # Implements getSpecification()
│   │
│   └── cli/
│       └── main.cpp                     # CLI entry point
│
└── CLI_SPEC.md                          # This document
```

---

## Benefits

### ✅ Automatic Documentation
- No manual documentation updates needed
- Specification always matches actual code
- Real-time configuration values shown

### ✅ Developer Productivity
- Instant reference for all modules and APIs
- Machine-readable (JSON) for tooling integration
- Human-readable (Markdown) for quick lookup

### ✅ Runtime Introspection
- See actual loaded configuration
- Check runtime statistics
- Debug module dependencies

### ✅ CI/CD Integration
```bash
# Generate spec in CI pipeline
cryo-engine-cli.exe spec --format json --output spec.json

# Validate against expected spec
diff spec.json expected-spec.json
```

### ✅ API Documentation Generation
```bash
# Generate HTML docs for web hosting
cryo-engine-cli.exe spec --format html --output docs/index.html
```

---

## Related Commands

```bash
# Show engine version
cryo-engine-cli.exe --version

# Show help (includes spec command)
cryo-engine-cli.exe --help

# Validate configuration without running
cryo-engine-cli.exe config validate

# List all registered modules
cryo-engine-cli.exe modules list

# Show specific module info
cryo-engine-cli.exe modules info RendererModule
```

---

**Cryo Engine CLI Specification** - Version 1.0.0