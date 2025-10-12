# Qimmiit Games

This directory contains complete game ecosystems built on the Qimmiit Engine.

## Structure

Each game is a self-contained ecosystem with:
- **Runtime** - Server and client implementations
- **Content** - Proto-based game data
- **Content Management** - Tools for managing game content
- **Configuration** - Environment-specific settings
- **Scripts** - Build and deployment automation
- **Docker** - Containerization for production

## Games

### avannaaq-001 (C++ / Native)
**Avannaaq** (Inuktitut: "Land, Territory")

A complete multiplayer game ecosystem featuring:
- **Technology**: C++20, CMake, Protocol Buffers
- **Platform**: Native desktop (Windows/Linux/macOS)
- Persistent game world with regions
- Item system (weapons, armor, consumables)
- NPC system (merchants, quest givers, enemies)
- TCP/UDP network multiplayer server
- Content management system
- Data-driven design using Protocol Buffers

See [avannaaq-001/README.md](avannaaq-001/README.md) for details.

### siqiniq-001 (Java / Quarkus)
**Siqiniq** (Inuktitut: "Sun")

A cloud-native game ecosystem built with modern Java:
- **Technology**: Java 21, Quarkus 3.x, Protocol Buffers
- **Platform**: Cloud-native (JVM + GraalVM Native)
- WebSocket real-time multiplayer (< 0.1s startup with Native)
- REST API for content management
- PostgreSQL persistence with Hibernate Panache
- Ultra-fast startup (~0.08s native, ~1.5s JVM)
- Low memory footprint (~50MB native, ~200MB JVM)
- Kubernetes-ready with health checks and metrics
- Hot reload development mode
- Built-in observability (Prometheus, OpenTelemetry)

See [siqiniq-001/README.md](siqiniq-001/README.md) and [siqiniq-001/QUICKSTART.md](siqiniq-001/QUICKSTART.md) for details.

## Creating a New Game

Each game follows this structure:

```
games/
└── your-game-name/
    ├── CMakeLists.txt           # Root build configuration
    ├── README.md                # Game documentation
    │
    ├── runtime/                 # Runtime components
    │   ├── server/              # Game server
    │   └── client/              # Game client
    │
    ├── content/                 # Static game content
    │   ├── proto/               # Content schemas
    │   ├── worlds/              # World data (.pbtxt)
    │   ├── items/               # Item data
    │   └── characters/          # Character/NPC data
    │
    ├── content-management/      # CMS tools
    │   ├── validation/          # Content validators
    │   └── tools/               # CLI tools
    │
    ├── config/                  # Configuration files
    │   ├── game.toml            # Core game settings
    │   ├── server_dev.json      # Dev server config
    │   └── server_prod.json     # Prod server config
    │
    ├── scripts/                 # Build scripts
    │   ├── build.bat            # Windows build
    │   └── build.sh             # Linux/macOS build
    │
    └── docker/                  # Containerization
        ├── Dockerfile.server
        └── docker-compose.yml
```

## Building a Game

Each game has its own build script:

**Windows:**
```bash
cd games/avannaaq-001
scripts\build.bat
```

**Linux/macOS:**
```bash
cd games/avannaaq-001
chmod +x scripts/build.sh
./scripts/build.sh
```

## Technology Choices

### C++ / Native (avannaaq-style)
**Best for:**
- Maximum performance
- Desktop games
- Low-level control
- Tight integration with Qimmiit Engine

**Pros:**
- Direct access to Qimmiit SDK
- Native performance
- Full control over memory and resources

**Cons:**
- Longer build times
- More complex deployment
- Manual memory management

### Java / Quarkus (siqiniq-style)
**Best for:**
- Cloud-native games
- Rapid development
- Microservices architecture
- Web-based games

**Pros:**
- Ultra-fast startup (GraalVM Native)
- Hot reload development
- Cloud-native features (health, metrics)
- Easy deployment (Docker, Kubernetes)
- Large ecosystem (Maven, JPA, etc.)

**Cons:**
- No direct Qimmiit rendering (client-side only)
- JVM overhead (mitigated with Native)

## Naming Convention

Games follow the Inuktitut naming theme:
- **avannaaq** - Land, Territory
- **siqiniq** - Sun
- **sila** - Weather, Environment, Universe
- **nunavut** - Our Land
- **anuri** - Wind
- **taqqiq** - Moon

Use descriptive Inuktitut names that reflect the game's theme or setting.

## Integration with Qimmiit Engine

All games use the Qimmiit SDK for:
- **Rendering** - Vulkan-based graphics (coregfx)
- **Assets** - Asset management (cryo-asset, cryo-cache)
- **Storage** - Persistent storage (akutik)
- **Tooling** - Development tools (cryo-tooling)

The SDK is automatically linked via:
```cmake
find_package(QimmiitSDKDev REQUIRED)
target_link_libraries(your-game PRIVATE Qimmiit::SDK)
```
