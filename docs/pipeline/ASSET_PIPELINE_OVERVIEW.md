# RPG Asset Content-Creation Pipeline
**Complete Automated Workflow for Game Asset Generation**

Copyright (c) 2025 Patrice Chevillat

---

## Overview

The **Qimmiit RPG Asset Pipeline** is a comprehensive, automated system for creating, processing, packing, and delivering game assets from high-performance C++/Vulkan renderers to multi-platform deployments (native and web).

### Key Features

✅ **High-Performance Rendering** - Vulkan GPU acceleration for asset generation
✅ **Automated Atlas Packing** - Intelligent sprite/texture atlas creation
✅ **Multi-Platform Delivery** - Native (C++) and Web (JavaScript) loaders
✅ **Efficient Packaging** - Compressed .pak bundles with streaming support
✅ **Memory-Optimized** - Stream processing, no full-scene loading required
✅ **Integrated Ecosystem** - Works seamlessly with cryo-asset, coregfx, siqiniq-001

---

## Pipeline Architecture

```
┌────────────────────────────────────────────────────────────────┐
│                    PIPELINE STAGES                             │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  1. ASSET GENERATION (C++ Vulkan Renderer)                     │
│     ┌──────────────────────────────────────┐                  │
│     │  cryo-render-cli                     │                  │
│     │  - Headless Vulkan rendering         │                  │
│     │  - Framebuffer capture               │                  │
│     │  - PNG export                        │                  │
│     └──────────────┬───────────────────────┘                  │
│                    │                                           │
│                    ▼                                           │
│     Output: .cache/render/*.png (individual sprites/tiles)     │
│                    │                                           │
├────────────────────┼───────────────────────────────────────────┤
│                    │                                           │
│  2. ATLAS PACKING (Texture Atlas Generation)                   │
│     ┌──────────────▼───────────────────────┐                  │
│     │  cryo-atlas-cli                      │                  │
│     │  - Rectangle packing                 │                  │
│     │  - Power-of-2 atlas generation       │                  │
│     │  - Metadata export                   │                  │
│     └──────────────┬───────────────────────┘                  │
│                    │                                           │
│                    ▼                                           │
│     Output: assets/atlases/*.png + *.meta.json                 │
│                    │                                           │
├────────────────────┼───────────────────────────────────────────┤
│                    │                                           │
│  3. ASSET PACKAGING (.pak Bundle Creation)                     │
│     ┌──────────────▼───────────────────────┐                  │
│     │  cryo-asset-cli (extended)           │                  │
│     │  - Scan asset directory              │                  │
│     │  - Compress assets (zstd)            │                  │
│     │  - Build PAK bundle                  │                  │
│     └──────────────┬───────────────────────┘                  │
│                    │                                           │
│                    ▼                                           │
│     Output: game_assets.pak (native)                           │
│             game_assets.web.pak (web)                          │
│                    │                                           │
├────────────────────┼───────────────────────────────────────────┤
│                    │                                           │
│  4. DEPLOYMENT & DELIVERY                                      │
│     ┌──────────────┴───────────────────────┐                  │
│     │                                       │                  │
│     ▼                                       ▼                  │
│  ┌─────────────────────┐     ┌──────────────────────┐         │
│  │  NATIVE LOADER      │     │  WEB LOADER          │         │
│  │  (C++)              │     │  (JavaScript)        │         │
│  │  - PakLoader        │     │  - WebPakLoader      │         │
│  │  - Arctic Renderer  │     │  - HTML5 Canvas/WebGL│         │
│  │  - Vulkan/coregfx   │     │  - Range requests    │         │
│  └─────────────────────┘     └──────────────────────┘         │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

## Tools

### 1. cryo-render-cli

**Headless Vulkan renderer for asset generation**

**Location:** `modules/rendering/cryo-render-cli/`

**Commands:**
```bash
# Render single asset
cryo-render-cli render --job jobs/player_sprite.json

# Batch render
cryo-render-cli batch --config jobs/all_assets.batch.json --parallel 8

# Generate sprite sheet
cryo-render-cli sprite-sheet --config jobs/walk_cycle.json

# Generate tileset
cryo-render-cli tileset --config jobs/dungeon_tiles.json

# List available GPUs
cryo-render-cli list-gpus

# Validate job
cryo-render-cli validate --job jobs/complex_scene.json
```

**Key Features:**
- Offscreen Vulkan rendering (no window/display required)
- High-performance GPU-accelerated rendering
- Batch processing with parallel execution
- Sprite sheet generation (animation frames)
- Tileset generation (RPG tile grids)
- Framebuffer capture to PNG
- Flexible JSON-based job definitions

**See:** [`docs/pipeline/CRYO_RENDER_CLI.md`](CRYO_RENDER_CLI.md)

---

### 2. cryo-atlas-cli

**Texture atlas packer for combining sprites**

**Location:** `modules/assets/cryo-atlas/`

**Commands:**
```bash
# Pack sprites into atlas
cryo-atlas-cli pack --config atlas_definitions/rpg_characters.json

# Auto-pack from directory
cryo-atlas-cli pack-dir --input .cache/render/sprites/ --output assets/atlases/auto_atlas.png

# Export metadata
cryo-atlas-cli export-meta --atlas assets/atlases/characters.png --format json

# Optimize atlas size
cryo-atlas-cli pack-optimized --config atlas_definitions/ui_elements.json
```

**Key Features:**
- Multiple packing algorithms (MaxRects, Guillotine, Skyline)
- Power-of-2 atlas sizes
- Sprite rotation for better packing
- Padding to prevent texture bleeding
- Multi-atlas support (when sprites don't fit in one)
- JSON/Protobuf metadata export
- Animation sequence definitions

**See:** [`docs/pipeline/CRYO_ATLAS_CLI.md`](CRYO_ATLAS_CLI.md)

---

### 3. cryo-asset-cli (Extended)

**Asset bundle packaging with PAK support**

**Location:** `modules/assets/cryo-asset/`

**New Commands:**
```bash
# Create PAK file
cryo-asset-cli pack --input assets/ --output game_assets.pak --compression zstd

# List PAK contents
cryo-asset-cli list-pak game_assets.pak

# Extract asset from PAK
cryo-asset-cli extract-pak game_assets.pak --asset character_atlas --output extracted/

# Verify PAK integrity
cryo-asset-cli verify-pak game_assets.pak --checksums

# Generate web-optimized PAK
cryo-asset-cli pack --input assets/ --output game_assets.web.pak --web-optimize
```

**Key Features:**
- Compressed asset bundles (zstd, lz4, gzip)
- Table of Contents (TOC) at file end for web streaming
- Range request support for partial downloads
- Dependency tracking
- Asset tagging and filtering
- Platform-specific variants
- Integrity verification (SHA256, CRC32)

**See:** [`docs/pipeline/CRYO_ASSET_CLI_PAK.md`](CRYO_ASSET_CLI_PAK.md)

---

### 4. cryo-pipeline-cli

**Pipeline orchestration tool**

**Location:** `modules/tools/cryo-pipeline-cli/`

**Commands:**
```bash
# Run full pipeline
cryo-pipeline-cli build --config pipeline_config.json

# Individual stages
cryo-pipeline-cli run-stage --stage render
cryo-pipeline-cli run-stage --stage pack-atlases
cryo-pipeline-cli run-stage --stage package

# Watch mode (rebuild on file changes)
cryo-pipeline-cli watch --config pipeline_config.json
```

**Key Features:**
- Multi-stage pipeline execution
- Dependency tracking between stages
- Parallel execution where possible
- Watch mode for development
- Progress reporting
- Error handling and recovery

**See:** [`docs/pipeline/CRYO_PIPELINE_CLI.md`](CRYO_PIPELINE_CLI.md)

---

## File Formats

### Job Definition (Render)

```json
{
  "job_id": "player_idle",
  "output_path": ".cache/render/player_idle.png",
  "width": 512,
  "height": 512,
  "scene_template": "scenes/character_template.arctic",
  "scene_parameters": {
    "character_color": "#FF0000",
    "animation": "idle",
    "weapon": "sword"
  },
  "camera": {
    "fov": 45.0,
    "position": [0, 1, -3],
    "target": [0, 0.5, 0]
  },
  "frame_count": 1,
  "enable_transparency": true,
  "metadata": {
    "sprite_type": "character",
    "variation": "red_knight"
  }
}
```

### Batch Configuration

```json
{
  "output_directory": ".cache/render/",
  "max_concurrent_jobs": 4,
  "fail_fast": false,
  "jobs": [
    "jobs/player_idle.json",
    "jobs/player_walk_01.json",
    "jobs/player_walk_02.json",
    "jobs/player_attack.json"
  ]
}
```

### Sprite Sheet Definition

```json
{
  "output_path": "assets/sprites/character_walk.png",
  "columns": 8,
  "rows": 1,
  "sprite_width": 128,
  "sprite_height": 128,
  "padding": 2,
  "frame_template": {
    "scene_template": "scenes/character.arctic",
    "width": 128,
    "height": 128,
    "enable_transparency": true
  },
  "frame_overrides": [
    {"scene_parameters": {"frame": 0}},
    {"scene_parameters": {"frame": 1}},
    {"scene_parameters": {"frame": 2}},
    {"scene_parameters": {"frame": 3}},
    {"scene_parameters": {"frame": 4}},
    {"scene_parameters": {"frame": 5}},
    {"scene_parameters": {"frame": 6}},
    {"scene_parameters": {"frame": 7}}
  ],
  "animations": [
    {
      "name": "walk",
      "start_frame": 0,
      "frame_count": 8,
      "duration_ms": 100,
      "loop": true
    }
  ],
  "metadata_output_path": "assets/sprites/character_walk.meta.json"
}
```

### Atlas Definition

```json
{
  "atlas_id": "rpg_characters",
  "output_path": "assets/atlases/characters.png",
  "width": 4096,
  "height": 4096,
  "padding": 2,
  "power_of_two": true,
  "algorithm": "MaxRects",
  "sprites": [
    {
      "sprite_id": "player_idle",
      "source_file": ".cache/render/player_idle.png",
      "tags": ["character", "player"]
    },
    {
      "sprite_id": "player_walk_01",
      "source_file": ".cache/render/player_walk_01.png",
      "animation": {
        "animation_name": "walk",
        "frame_count": 8,
        "frame_duration_ms": 100
      }
    }
  ],
  "metadata_output_path": "assets/atlases/characters.meta.json",
  "metadata_format": "JSON"
}
```

### Atlas Metadata (Output)

```json
{
  "atlas_id": "rpg_characters",
  "width": 4096,
  "height": 4096,
  "sprites": {
    "player_idle": {
      "x": 0,
      "y": 0,
      "width": 128,
      "height": 128,
      "u0": 0.0,
      "v0": 0.0,
      "u1": 0.03125,
      "v1": 0.03125
    },
    "player_walk_01": {
      "x": 128,
      "y": 0,
      "width": 128,
      "height": 128,
      "u0": 0.03125,
      "v0": 0.0,
      "u1": 0.0625,
      "v1": 0.03125,
      "animation": {
        "animation_name": "walk",
        "frame_count": 8,
        "frame_duration_ms": 100
      }
    }
  }
}
```

### Pipeline Configuration

```json
{
  "pipeline": {
    "name": "RPG Asset Pipeline",
    "version": "1.0.0",
    "stages": [
      {
        "name": "render",
        "tool": "cryo-render-cli",
        "command": "batch",
        "args": ["--config", "jobs/all_assets.batch.json", "--parallel", "8"],
        "input": "jobs/*.json",
        "output": ".cache/render/",
        "depends_on": []
      },
      {
        "name": "pack_atlases",
        "tool": "cryo-atlas-cli",
        "command": "pack",
        "args": ["--config", "atlas_definitions/characters.json"],
        "input": ".cache/render/",
        "output": "assets/atlases/",
        "depends_on": ["render"]
      },
      {
        "name": "package_native",
        "tool": "cryo-asset-cli",
        "command": "pack",
        "args": ["--input", "assets/", "--output", "game_assets.pak", "--compression", "zstd"],
        "input": "assets/",
        "output": "game_assets.pak",
        "depends_on": ["pack_atlases"]
      },
      {
        "name": "package_web",
        "tool": "cryo-asset-cli",
        "command": "pack",
        "args": ["--input", "assets/", "--output", "game_assets.web.pak", "--web-optimize"],
        "input": "assets/",
        "output": "game_assets.web.pak",
        "depends_on": ["pack_atlases"]
      }
    ]
  }
}
```

---

## Usage Examples

### Complete Workflow Example

```bash
# 1. Generate assets using Vulkan renderer
cryo-render-cli batch --config jobs/rpg_assets.batch.json --parallel 8

# 2. Pack sprites into texture atlases
cryo-atlas-cli pack --config atlas_definitions/characters.json
cryo-atlas-cli pack --config atlas_definitions/tiles.json
cryo-atlas-cli pack --config atlas_definitions/ui.json

# 3. Package assets into PAK bundles
cryo-asset-cli pack --input assets/ --output game_assets.pak --compression zstd
cryo-asset-cli pack --input assets/ --output game_assets.web.pak --web-optimize

# 4. Verify integrity
cryo-asset-cli verify-pak game_assets.pak --checksums
```

### Or use the pipeline tool:

```bash
# Run full pipeline
cryo-pipeline-cli build --config pipeline_config.json

# Watch mode for development
cryo-pipeline-cli watch --config pipeline_config.json
```

---

## Integration with Existing Tools

### With siqiniq-001 (Render Stream Server)

The render server can optionally stream rendered frames for live preview during asset generation:

```proto
service AssetGenerationService {
    rpc StreamAssetGeneration(AssetGenerationRequest) returns (stream AssetFrame);
    rpc GetRenderJobStatus(RenderJobId) returns (RenderJobStatus);
}
```

### With cryo-asset (Asset Loading)

The existing `AssetLoader` is extended to support PAK files:

```cpp
auto pak_loader = std::make_unique<cryo::asset::PakLoader>();
pak_loader->mount("game_assets.pak");

auto atlas_png = pak_loader->loadAsset("rpg_atlas_01");
auto atlas_meta = pak_loader->loadAsset("rpg_atlas_01_meta");
```

### With coregfx (Rendering)

Load and use texture atlases in your Vulkan renderer:

```cpp
// Load atlas from PAK
VkImage atlas_texture = loadTextureFromPak("rpg_characters");
AtlasMetadata meta = AtlasMetadata::fromJSON(
    pak_loader->loadAsset("rpg_characters_meta")
);

// Use sprite from atlas
const auto* sprite = meta.getSprite("player_idle");
drawSprite(atlas_texture, sprite->u0, sprite->v0, sprite->u1, sprite->v1);
```

---

## Performance Characteristics

### Rendering (cryo-render-cli)

- **Throughput:** ~50-100 sprites/second (1024x1024)
- **Memory:** ~500MB GPU VRAM for offscreen rendering
- **Parallelization:** Full GPU parallelism per job
- **Best for:** Batch processing thousands of assets

### Atlas Packing (cryo-atlas-cli)

- **Speed:** ~1000 sprites/second (packing only)
- **Memory:** ~200MB RAM for 4096x4096 atlas
- **Efficiency:** 85-95% packing efficiency with MaxRects
- **Best for:** Combining hundreds of sprites into unified textures

### PAK Loading (Runtime)

- **Native (C++):** ~500MB/s decompression (zstd)
- **Web (JS):** ~100MB/s with range requests
- **Memory:** Minimal - stream decompression
- **Latency:** <10ms for single asset load from PAK

---

## Directory Structure

```
qimmiit/
├── modules/
│   ├── rendering/
│   │   └── cryo-render-cli/              # Vulkan asset renderer
│   │       ├── include/cryo_render/
│   │       ├── src/
│   │       ├── jobs/                     # Job definitions
│   │       └── CMakeLists.txt
│   ├── assets/
│   │   ├── cryo-asset/                   # Asset loading (extended with PAK)
│   │   │   ├── include/cryo_asset/pak/
│   │   │   └── src/pak/
│   │   └── cryo-atlas/                   # Atlas packer
│   │       ├── include/cryo_atlas/
│   │       ├── src/
│   │       └── CMakeLists.txt
│   ├── tools/
│   │   └── cryo-pipeline-cli/            # Pipeline orchestration
│   └── formats/
│       └── arctic-pak/
│           └── proto/
│               └── arctic_pak.proto      # PAK format definition
├── web/
│   └── asset-loader/
│       └── WebPakLoader.js               # Web loader
├── docs/
│   └── pipeline/
│       ├── ASSET_PIPELINE_OVERVIEW.md    # This file
│       ├── CRYO_RENDER_CLI.md
│       ├── CRYO_ATLAS_CLI.md
│       ├── CRYO_ASSET_CLI_PAK.md
│       └── CRYO_PIPELINE_CLI.md
└── assets/                               # Generated assets
    ├── atlases/                          # Texture atlases
    ├── models/                           # 3D models
    ├── scenes/                           # Scene definitions
    └── .cache/
        └── render/                       # Intermediate render output
```

---

## Next Steps

1. **Build the tools:**
   ```bash
   cd modules/rendering/cryo-render-cli
   cmake -B build -S .
   cmake --build build
   ```

2. **Create job definitions** for your RPG assets

3. **Run the pipeline:**
   ```bash
   cryo-pipeline-cli build --config pipeline_config.json
   ```

4. **Integrate with your game:**
   - Native: Use `PakLoader` to mount and load assets
   - Web: Use `WebPakLoader` for streaming delivery

---

## See Also

- [cryo-render-cli Documentation](CRYO_RENDER_CLI.md)
- [cryo-atlas-cli Documentation](CRYO_ATLAS_CLI.md)
- [PAK Format Specification](CRYO_ASSET_CLI_PAK.md)
- [Pipeline Configuration Guide](CRYO_PIPELINE_CLI.md)
- [Web Deployment Guide](WEB_DEPLOYMENT.md)

---

**Copyright (c) 2025 Patrice Chevillat. All Rights Reserved.**