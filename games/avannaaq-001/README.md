# Avannaaq-001 Game Ecosystem

**Avannaaq** (Inuktitut: "Land, Territory") - A complete game ecosystem built on the Qimmiit Engine.

## Overview

This is a self-contained game ecosystem that includes:
- **Runtime Server** - Game server with networking, gameplay logic, and state management
- **Runtime Client** - Game-specific client built on qimmiit-renderer-client
- **Static Content** - Proto-based game data (worlds, items, characters)
- **Content Management** - CMS for managing and validating game content
- **Configuration** - Environment-specific configurations

## Architecture

```
avannaaq-001/
├── runtime/              # Game runtime (server + client)
├── content/              # Static game content (proto-based)
├── content-management/   # Dynamic content CMS
├── config/               # Game configuration
├── scripts/              # Build and deployment scripts
└── docker/               # Containerization
```

## Building

### Prerequisites
- Qimmiit SDK installed
- CMake 3.20+
- vcpkg (for dependencies)
- Protobuf compiler

### Build Steps

```bash
# Configure
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build all components
cmake --build build --config Release

# Build specific components
cmake --build build --target avannaaq-server
cmake --build build --target avannaaq-client
cmake --build build --target avannaaq-content
```

## Running

### Development Setup

```bash
# 1. Start the game server
./build/bin/avannaaq-server --config config/server_dev.json

# 2. Start the client
./build/bin/avannaaq-client --server localhost:8080
```

### Production Deployment

```bash
# Using Docker
cd docker
docker-compose up -d
```

## Content Workflow

### Creating Content

1. Edit `.pbtxt` files in `content/`:
   ```bash
   vim content/worlds/overworld.pbtxt
   ```

2. Compile content to binary:
   ```bash
   cmake --build build --target compile-content
   ```

3. Validate content:
   ```bash
   ./build/bin/content-validator --input content/
   ```

### Content Management

The CMS provides tools for managing game content:

```bash
# List all content
./build/bin/content-cli list

# Validate content
./build/bin/content-cli validate --path content/worlds/

# Migrate content to new schema version
./build/bin/content-cli migrate --from 1.0 --to 1.1
```

## Game Features

- **Persistent World** - Server maintains game state using akutik storage
- **Network Multiplayer** - TCP/UDP networking with packet handling
- **Data-Driven Design** - All game content defined in proto files
- **Hot Reloading** - Content changes reflected without restart
- **Content Versioning** - Automatic migration of content schemas

## Configuration

Configuration files are located in `config/`:
- `game.toml` - Core game settings
- `server_dev.json` - Development server config
- `server_prod.json` - Production server config

## Development

### Adding New Content

1. Define proto schema in `content/proto/`
2. Create `.pbtxt` data files
3. Add to `content/CMakeLists.txt`
4. Rebuild content

### Adding New Game Systems

1. Implement in `runtime/server/src/gameplay/`
2. Link with Qimmiit SDK modules
3. Add configuration in `config/game.toml`

## Technology Stack

- **Engine**: Qimmiit Engine (Vulkan rendering, asset management)
- **Storage**: Akutik (SQLite-based storage backend)
- **Content**: Protocol Buffers (schema + data)
- **Networking**: Custom TCP/UDP with gRPC for services
- **Scripting**: C++20 with data-driven design

## License

[Your License]
