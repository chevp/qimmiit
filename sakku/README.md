# Sakku - Library Registry CLI

**Sakku** (Inuktitut: "cache" or "storage") is a package manager for game content libraries. Like npm for JavaScript or crates.io for Rust, Sakku provides a centralized registry for game assets, models, shaders, and other reusable content.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Overview

Sakku allows game developers to:
- 🔍 **Search** for reusable game content libraries
- 📦 **Install** libraries directly into their projects
- 🚀 **Publish** their own content for others to use
- 🔄 **Manage** library versions and dependencies

## Features

- **Registry Integration**: Search and download from remote registries
- **Version Management**: Semantic versioning support with `@latest` tags
- **Simple CLI**: Intuitive command-line interface
- **Manifest System**: JSON-based library metadata and asset tracking
- **Offline Support**: Install from local directories or archives
- **Dependency Resolution**: Automatic dependency handling

## Installation

### Build from Source

```bash
# Prerequisites
cmake --version  # Requires CMake 3.20+
vcpkg install curl nlohmann-json

# Build
cd sakku
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# The executable will be in build/bin/sakku-cli
```

### Add to PATH

```bash
# Windows
set PATH=%PATH%;C:\path\to\sakku\build\bin

# Linux/macOS
export PATH=$PATH:/path/to/sakku/build/bin
```

## Quick Start

### Search for Libraries

```bash
# Search for industrial content
sakku search "industrial containers"

# Search with tag filter
sakku search "sci-fi" --tag 3d-models

# Paginated results
sakku search "environment" --page 2 --limit 20
```

### Install a Library

```bash
# Install latest version
sakku add industrial-pack@latest

# Install specific version
sakku add sci-fi-assets@1.2.0

# Install to custom directory
sakku add medieval-pack --output ./game-assets
```

### Publish Your Library

```bash
# Publish to default registry
sakku publish ./my-library --token YOUR_API_TOKEN

# Dry run (validate without publishing)
sakku publish ./my-library --dry-run

# Custom registry
sakku publish ./my-library --registry https://custom-registry.io --token TOKEN
```

## Library Structure

A library should have the following structure:

```
my-library/
├── sakku.json          # Library manifest (required)
├── assets/             # Asset files
│   ├── models/
│   │   ├── container.fbx
│   │   └── container.gltf
│   ├── textures/
│   │   ├── container_diffuse.png
│   │   └── container_normal.png
│   └── materials/
│       └── container.mat
└── README.md          # Documentation (optional)
```

## Manifest Format

Create a `sakku.json` file in your library:

```json
{
  "name": "industrial-pack",
  "version": "1.0.0",
  "description": "Industrial 3D assets including containers, crates, and machinery",
  "author": "Your Name <email@example.com>",
  "tags": ["3d-models", "industrial", "pbr", "game-ready"],
  "dependencies": [
    "common-materials@^2.0.0",
    "shader-library@1.5.0"
  ],
  "metadata": {
    "license": "MIT",
    "homepage": "https://github.com/yourname/industrial-pack"
  },
  "assets": [
    {
      "path": "assets/models/container.gltf",
      "type": "model",
      "size": 245678,
      "hash": "abc123def456"
    },
    {
      "path": "assets/textures/container_diffuse.png",
      "type": "texture",
      "size": 1048576,
      "hash": "def789ghi012"
    }
  ]
}
```

## Registry API

Sakku communicates with registries using a REST API:

### Endpoints

```
GET  /api/search?q=<query>&page=<n>&limit=<n>
GET  /api/packages/<name>
GET  /api/packages/<name>/<version>
GET  /api/packages/<name>/latest
GET  /api/packages/<name>/<version>/manifest
POST /api/packages (publish)
```

### Response Format

```json
{
  "name": "industrial-pack",
  "version": "1.0.0",
  "description": "Industrial 3D assets",
  "author": "Developer Name",
  "tags": ["3d-models", "industrial"],
  "downloadUrl": "https://cdn.sakku.io/libraries/industrial-pack-1.0.0.zip",
  "manifestUrl": "https://cdn.sakku.io/libraries/industrial-pack/v1.0.0/manifest.json",
  "downloads": 1234,
  "createdAt": "2024-01-15T10:30:00Z",
  "updatedAt": "2024-03-20T14:45:00Z"
}
```

## Environment Variables

Configure Sakku using environment variables:

```bash
# Default registry URL
export SAKKU_REGISTRY_URL=https://registry.sakku.io

# API authentication token
export SAKKU_API_TOKEN=your_api_token_here

# Default installation directory
export SAKKU_LIBRARIES_PATH=./libraries
```

## Commands Reference

### `sakku search <query>`

Search for libraries in the registry.

**Options:**
- `--page <number>` - Page number (default: 1)
- `--limit <number>` - Results per page (default: 10)
- `--tag <tag>` - Filter by tag
- `--registry <url>` - Custom registry URL

**Example:**
```bash
sakku search "sci-fi weapons" --tag 3d-models --limit 20
```

### `sakku add <name>[@version]`

Install a library from the registry.

**Options:**
- `--output <path>` - Installation directory (default: ./libraries)
- `--registry <url>` - Custom registry URL

**Example:**
```bash
sakku add industrial-pack@1.2.0 --output ./game-content
```

### `sakku publish <path>`

Publish a library to the registry.

**Options:**
- `--token <token>` - API authentication token (or use SAKKU_API_TOKEN env var)
- `--registry <url>` - Custom registry URL
- `--dry-run` - Validate without publishing

**Example:**
```bash
sakku publish ./my-library --token abc123 --dry-run
```

### `sakku version`

Show version information.

### `sakku help`

Show help message with all commands and options.

## Use Cases

### Game Content Library

Share your curated collection of game-ready assets:

```bash
# Create library
mkdir space-station-pack
cd space-station-pack

# Add your assets
mkdir -p assets/{models,textures,materials}
# ... copy your asset files ...

# Create manifest
cat > sakku.json << EOF
{
  "name": "space-station-pack",
  "version": "1.0.0",
  "description": "Sci-fi space station modular kit",
  "author": "Your Studio",
  "tags": ["3d-models", "sci-fi", "modular", "space"]
}
EOF

# Publish
sakku publish . --token YOUR_TOKEN
```

### Shader Library

Share reusable shaders:

```bash
# Create shader library
mkdir pbr-shaders
cd pbr-shaders

cat > sakku.json << EOF
{
  "name": "pbr-shaders",
  "version": "2.1.0",
  "description": "Physically-based rendering shaders for game engines",
  "author": "Graphics Team",
  "tags": ["shaders", "pbr", "rendering", "vulkan"]
}
EOF

# Publish
sakku publish .
```

### Material Library

Create a library of materials:

```bash
sakku search materials --tag pbr
sakku add standard-materials@latest
```

## Architecture

Sakku is built with:

- **C++17**: Modern C++ for performance and portability
- **libcurl**: HTTP client for registry communication
- **nlohmann/json**: JSON parsing and serialization
- **std::filesystem**: Cross-platform file operations

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

[License information]

## Related Projects

Part of the Qimmiit game engine ecosystem:

- **[Qimmiit](../README.md)**: Main game engine
- **[Nanook](../nanook/README.md)**: Module orchestrator
- **[Akutik](../akutik/README.md)**: Asset management system

## Support

- Documentation: https://sakku.io/docs
- Issues: https://github.com/yourorg/sakku/issues
- Registry: https://registry.sakku.io

---

**Sakku v1.0.0** - Library Registry for Game Content
*"Like npm/crates.io for game development"*
