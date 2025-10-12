# Sakku - Game Content Management CLI

**Sakku** (Inuktitut: "cache" or "storage") is a content management CLI tool for game developers. Like Angular CLI (`ng`) or Vue CLI, Sakku is installed via npm and helps you manage game content packages from the npm registry.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Node.js](https://img.shields.io/badge/Node.js-18+-green)
![License](https://img.shields.io/badge/license-MIT-green)

## Overview

Sakku allows game developers to:
- 🔍 **Search** for game content packages on npm
- 📦 **Install** content libraries (models, textures, shaders) from npm
- 🎮 **Create** new game projects with pre-configured content
- 🛠️ **Build** and optimize game assets for production
- 🚀 **Publish** your own content packages to npm

## Key Concept

Sakku is **NOT** a package manager itself. Instead, it's a **hybrid CLI tool** that combines:

### Node.js Wrapper (`@qimmiit/sakku`)
- Installed via npm (like `ng`, `create-react-app`, or `vue-cli`)
- Wraps npm commands for content package management
- Provides project scaffolding and validation
- Works with standard npm packages

### Native C++ Binary (`sakku-cli.exe`)
- High-performance asset processing
- GLTF optimization, texture compression, shader compilation
- Called internally by the Node.js wrapper
- Bundled with the npm package (platform-specific binaries)

## Installation

Install Sakku globally via npm:

```bash
# Install globally
npm install -g @qimmiit/sakku

# Or use with npx (no installation needed)
npx @qimmiit/sakku create my-game
```

### Prerequisites

- Node.js 18+ and npm 9+
- Native binaries (`sakku-cli.exe`) are bundled automatically for Windows/Linux/macOS
- Optional: Build from source requires CMake 3.20+ and vcpkg

## Quick Start

### Create a New Game Project

```bash
# Create a new game project with starter content
sakku create my-awesome-game

# Create with specific template
sakku create my-fps --template fps-starter

# Create in current directory
sakku create . --template minimal
```

### Add Content Packages

```bash
# Search for content on npm
sakku search @qimmiit-content/industrial

# Add content package from npm
sakku add @qimmiit-content/industrial-pack

# Add specific version
sakku add @qimmiit-content/sci-fi-assets@1.2.0

# This internally runs: npm install @qimmiit-content/industrial-pack
```

### Build and Optimize Assets

```bash
# Build all assets for production
# (Node.js wrapper calls sakku-cli.exe internally)
sakku build

# Build with optimization (native C++ processing)
sakku build --optimize --compress

# Watch mode for development
sakku build --watch
```

### Publish Your Content Package

```bash
# Initialize package.json if needed
sakku init

# Publish to npm (uses npm publish internally)
npm publish

# Or use sakku wrapper for game-specific validation
sakku publish
```

## Content Package Structure

A content package is a standard npm package with game assets:

```
@qimmiit-content/my-pack/
├── package.json           # Standard npm package.json
├── qimmiit.json          # Optional: Qimmiit-specific metadata
├── assets/               # Asset files
│   ├── models/
│   │   ├── container.gltf
│   │   └── container.fbx
│   ├── textures/
│   │   ├── diffuse.png
│   │   └── normal.png
│   └── materials/
│       └── container.mat
└── README.md
```

## Package.json Format

Standard npm `package.json` with optional Qimmiit metadata:

```json
{
  "name": "@qimmiit-content/industrial-pack",
  "version": "1.0.0",
  "description": "Industrial 3D assets for games",
  "author": "Your Name <email@example.com>",
  "license": "MIT",
  "keywords": ["game-assets", "3d-models", "industrial", "qimmiit"],
  "main": "index.js",
  "files": [
    "assets/**/*",
    "qimmiit.json"
  ],
  "qimmiit": {
    "type": "content-pack",
    "engine": "^1.0.0",
    "assets": {
      "models": "assets/models",
      "textures": "assets/textures",
      "materials": "assets/materials"
    }
  },
  "peerDependencies": {
    "@qimmiit-content/common-materials": "^2.0.0"
  }
}
```

## How Sakku Works with npm

Sakku wraps npm commands and adds game-specific functionality:

| Sakku Command | What It Does | npm Equivalent |
|--------------|--------------|----------------|
| `sakku add pkg` | Installs content + validates | `npm install pkg` |
| `sakku search term` | Searches npm for game content | `npm search term` |
| `sakku publish` | Validates + publishes to npm | `npm publish` |
| `sakku build` | Processes game assets | (custom) |
| `sakku create` | Scaffolds new project | (custom) |

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

Sakku uses a **hybrid architecture** combining Node.js and C++:

### Node.js Layer (`@qimmiit/sakku`)
- **Node.js 18+**: JavaScript runtime
- **npm**: Package manager (used internally)
- **Commander.js**: CLI framework
- **Inquirer.js**: Interactive prompts
- **Chalk**: Terminal styling
- Handles package management, scaffolding, and orchestration

### Native C++ Layer (`sakku-cli.exe`)
- **C++17**: High-performance asset processing
- **libcurl**: HTTP downloads (optional)
- **nlohmann/json**: JSON parsing
- **std::filesystem**: File operations
- **Built with CMake + vcpkg**

### How They Work Together

```
User Command: sakku build --optimize

┌─────────────────────────────────┐
│  Node.js CLI (@qimmiit/sakku)  │
│  - Parse command line args      │
│  - Validate configuration       │
│  - Find sakku-cli.exe binary   │
└─────────────┬───────────────────┘
              │ spawns
              ▼
┌─────────────────────────────────┐
│   Native Binary (sakku-cli.exe) │
│   - Process GLTF models         │
│   - Compress textures           │
│   - Compile shaders             │
│   - Optimize meshes             │
└─────────────────────────────────┘
```

**Benefits:**
- **npm integration**: Standard JavaScript tooling for package management
- **Native performance**: C++ for intensive asset processing
- **Cross-platform**: Bundled binaries for all major platforms
- **Best of both worlds**: Easy distribution (npm) + fast execution (C++)

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

[License information]

## Related Projects

Part of the Qimmiit game engine ecosystem:

- **[Qimmiit](../../README.md)**: Main game engine
- **[Nanook](../../orchestrators/nanook/README.md)**: Module orchestrator
- **[Akutik](../../modules/toolings/akutik/README.md)**: Asset management system
- **[Qilak Quickstarts](../../qilak-quickstarts/README.md)**: Example projects and tutorials

## Support

- Documentation: https://sakku.io/docs
- Issues: https://github.com/yourorg/sakku/issues
- Registry: https://registry.sakku.io

---

**Sakku v1.0.0** - Content Management CLI for Game Development
*"Like Angular CLI for game content - built on npm"*
