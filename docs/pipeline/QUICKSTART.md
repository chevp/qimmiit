# RPG Asset Pipeline - Quick Start Guide

Get up and running with the asset pipeline in 15 minutes!

---

## Prerequisites

- CMake 3.20+
- C++20 compiler (MSVC, GCC, Clang)
- Vulkan SDK
- vcpkg (for dependencies)

---

## Step 1: Build the Tools

```bash
cd modules/rendering/cryo-render-cli
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release

cd ../../assets/cryo-atlas
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

**Verify installation:**

```bash
./build/Release/cryo-render-cli help
./build/Release/cryo-atlas-cli help
```

---

## Step 2: Create Your First Render Job

Create `jobs/player_sprite.json`:

```json
{
  "job_id": "player_idle",
  "output_path": ".cache/render/player_idle.png",
  "width": 512,
  "height": 512,
  "scene_template": "scenes/character_template.arctic",
  "scene_parameters": {
    "character_model": "models/knight.glb",
    "animation": "idle",
    "color_variant": "blue"
  },
  "camera": {
    "fov": 45.0,
    "position": [0, 1, -3],
    "target": [0, 0.5, 0]
  },
  "enable_transparency": true
}
```

---

## Step 3: Render Your First Asset

```bash
cryo-render-cli render --job jobs/player_sprite.json
```

**Output:** `.cache/render/player_idle.png`

---

## Step 4: Create Multiple Assets (Batch)

Create `jobs/all_characters.batch.json`:

```json
{
  "output_directory": ".cache/render/",
  "max_concurrent_jobs": 4,
  "jobs": [
    "jobs/player_idle.json",
    "jobs/player_walk_01.json",
    "jobs/player_walk_02.json",
    "jobs/player_walk_03.json",
    "jobs/player_walk_04.json",
    "jobs/player_attack.json",
    "jobs/enemy_idle.json",
    "jobs/enemy_attack.json"
  ]
}
```

```bash
cryo-render-cli batch --config jobs/all_characters.batch.json --parallel 8
```

---

## Step 5: Pack Sprites into Atlas

Create `atlas_definitions/characters.json`:

```json
{
  "atlas_id": "rpg_characters",
  "output_path": "assets/atlases/characters.png",
  "width": 4096,
  "height": 4096,
  "padding": 2,
  "algorithm": "MaxRects",
  "auto_discovery": {
    "enabled": true,
    "input_directory": ".cache/render/",
    "file_pattern": "*.png",
    "recursive": false
  },
  "metadata_output_path": "assets/atlases/characters.meta.json",
  "metadata_format": "JSON"
}
```

```bash
cryo-atlas-cli pack --config atlas_definitions/characters.json
```

**Output:**
- `assets/atlases/characters.png` (4096x4096 texture atlas)
- `assets/atlases/characters.meta.json` (sprite coordinates)

---

## Step 6: Package into PAK Bundle

```bash
cryo-asset-cli pack \
  --input assets/ \
  --output game_assets.pak \
  --compression zstd
```

**Output:** `game_assets.pak` (compressed bundle)

---

## Step 7: Use in Your Game (Native C++)

```cpp
#include <cryo_asset/pak/PakLoader.hpp>

int main() {
    // Load PAK
    auto loader = cryo::asset::PakLoaderFactory::createAndMount("game_assets.pak");

    // Load texture atlas
    auto atlas_png = loader->loadAsset("rpg_characters");
    auto atlas_meta_data = loader->loadAsset("rpg_characters_meta");

    // Parse metadata
    std::string meta_json(atlas_meta_data.begin(), atlas_meta_data.end());
    auto metadata = nlohmann::json::parse(meta_json);

    // Get sprite coordinates
    auto player_idle = metadata["sprites"]["player_idle"];
    int x = player_idle["x"];
    int y = player_idle["y"];
    int width = player_idle["width"];
    int height = player_idle["height"];

    // Create Vulkan texture from PNG data
    VkImage texture = createTextureFromPNG(atlas_png.data(), atlas_png.size());

    // Render sprite...

    return 0;
}
```

---

## Step 8: Use in Web (JavaScript)

```html
<!DOCTYPE html>
<html>
<head>
    <title>RPG Game</title>
</head>
<body>
    <canvas id="game" width="800" height="600"></canvas>

    <script src="WebPakLoader.js"></script>
    <script>
        (async () => {
            const canvas = document.getElementById('game');
            const ctx = canvas.getContext('2d');

            // Load PAK
            const loader = new WebPakLoader('game_assets.web.pak');
            await loader.init();

            // Load texture atlas
            const atlas = await loader.loadAtlas('rpg_characters');

            // Get sprite
            const playerSprite = atlas.getSprite('player_idle');

            // Render sprite
            function render() {
                ctx.clearRect(0, 0, canvas.width, canvas.height);

                ctx.drawImage(
                    atlas.image,
                    playerSprite.x, playerSprite.y,
                    playerSprite.width, playerSprite.height,
                    100, 100,
                    playerSprite.width, playerSprite.height
                );

                requestAnimationFrame(render);
            }

            render();
        })();
    </script>
</body>
</html>
```

---

## Complete Workflow (Automated)

Create `pipeline_config.json`:

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
        "args": ["--config", "jobs/all_characters.batch.json", "--parallel", "8"]
      },
      {
        "name": "pack_atlases",
        "tool": "cryo-atlas-cli",
        "command": "pack",
        "args": ["--config", "atlas_definitions/characters.json"],
        "depends_on": ["render"]
      },
      {
        "name": "package",
        "tool": "cryo-asset-cli",
        "command": "pack",
        "args": ["--input", "assets/", "--output", "game_assets.pak", "--compression", "zstd"],
        "depends_on": ["pack_atlases"]
      }
    ]
  }
}
```

```bash
cryo-pipeline-cli build --config pipeline_config.json
```

**Done!** Your assets are now rendered, packed, and bundled.

---

## Next Steps

1. **Create more complex scenes** with animations
2. **Generate tilesets** for RPG maps
3. **Optimize atlas packing** for better efficiency
4. **Add custom metadata** to your assets
5. **Deploy to web** with streaming PAK loader

See the full documentation:
- [Asset Pipeline Overview](ASSET_PIPELINE_OVERVIEW.md)
- [cryo-render-cli Guide](CRYO_RENDER_CLI.md)
- [cryo-atlas-cli Guide](CRYO_ATLAS_CLI.md)
- [PAK Format Specification](CRYO_ASSET_CLI_PAK.md)

---

**Copyright (c) 2025 Patrice Chevillat**