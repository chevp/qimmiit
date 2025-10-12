# Siqiniq-001 Game Ecosystem

**Siqiniq** (Inuktitut: "Sun") - A complete Java Quarkus-based game ecosystem built on modern cloud-native architecture.

## Overview

This is a cloud-native game ecosystem built with:
- **Quarkus** - Supersonic Subatomic Java framework
- **Protocol Buffers** - For game content and networking
- **WebSocket** - Real-time multiplayer communication
- **REST API** - Content management and admin
- **PostgreSQL** - Persistent game state
- **GraalVM Native** - Ultra-fast startup and low memory

## Architecture

```
siqiniq-001/
├── src/main/java/          # Java source code
│   └── io/qimmiit/siqiniq/
│       ├── server/         # Game server (WebSocket)
│       └── cms/            # Content Management System (REST)
├── src/main/resources/     # Configuration and resources
├── src/main/proto/         # Proto definitions
├── content/                # Game content (.pbtxt files)
├── config/                 # Environment configs
├── scripts/                # Build scripts
└── docker/                 # Container images
```

## Technology Stack

- **Framework**: Quarkus 3.x (Java 21)
- **Build**: Maven
- **Content**: Protocol Buffers
- **Real-time**: WebSocket (jakarta.websocket)
- **REST**: JAX-RS (RESTEasy)
- **Database**: PostgreSQL + Hibernate/Panache
- **Native**: GraalVM Native Image
- **Container**: Docker + Kubernetes ready

## Features

- ☀️ **Cloud-Native** - Designed for containers and Kubernetes
- ⚡ **Fast Startup** - < 0.1s with GraalVM Native
- 🔌 **WebSocket Multiplayer** - Real-time game networking
- 🎮 **Data-Driven** - All content in Proto files
- 🔄 **Hot Reload** - Dev mode with live coding
- 📊 **Observability** - Metrics, health checks, tracing
- 🔐 **Security** - JWT authentication ready

## Quick Start

### Prerequisites

- Java 21+
- Maven 3.9+
- Docker (optional)
- GraalVM (for native builds)

### Development Mode

```bash
# Run in dev mode with live reload
./mvnw quarkus:dev

# Server will start on http://localhost:8080
# Dev UI available at http://localhost:8080/q/dev/
```

### Building

```bash
# JVM build
./mvnw clean package

# Native build (requires GraalVM)
./mvnw clean package -Pnative

# Docker image
./mvnw clean package -Pnative -Dquarkus.container-image.build=true
```

### Running

```bash
# JVM mode
java -jar target/quarkus-app/quarkus-run.jar

# Native mode
./target/siqiniq-001-runner

# Docker
docker run -p 8080:8080 qimmiit/siqiniq-001:1.0.0
```

## API Endpoints

### WebSocket (Game Server)
- `ws://localhost:8080/game/ws` - Game multiplayer connection

### REST API (Content Management)
- `GET /api/worlds` - List all worlds
- `GET /api/items` - List all items
- `GET /api/characters` - List all characters
- `POST /api/content/validate` - Validate content

### Health & Metrics
- `GET /q/health` - Health check
- `GET /q/metrics` - Prometheus metrics
- `GET /q/openapi` - OpenAPI spec

## Configuration

Configuration files in `src/main/resources/`:
- `application.properties` - Default config
- `application-dev.properties` - Development
- `application-prod.properties` - Production

Environment variables:
```bash
QUARKUS_HTTP_PORT=8080
QUARKUS_DATASOURCE_JDBC_URL=jdbc:postgresql://localhost:5432/siqiniq
SIQINIQ_GAME_MAX_PLAYERS=100
```

## Content Workflow

### 1. Define Content Schema
Edit proto files in `src/main/proto/`:
```protobuf
// world.proto
message World {
  string id = 1;
  string name = 2;
  // ...
}
```

### 2. Create Content Data
Create `.pbtxt` files in `content/`:
```
# content/worlds/sunny_plains.pbtxt
id: "sunny_plains_001"
name: "Sunny Plains"
```

### 3. Validate Content
```bash
curl -X POST http://localhost:8080/api/content/validate \
  -H "Content-Type: application/json" \
  -d '{"path": "content/"}'
```

### 4. Load in Game
Content is automatically loaded on server startup and available via API.

## Development

### Hot Reload
Start dev mode and edit code - changes are reflected immediately:
```bash
./mvnw quarkus:dev
```

### Testing
```bash
# Run all tests
./mvnw test

# Integration tests
./mvnw verify

# With coverage
./mvnw verify -Pjacoco
```

### Native Image
Build a native executable for ultra-fast startup:
```bash
./mvnw package -Pnative
./target/siqiniq-001-runner

# Startup time: ~0.08s
# Memory: ~50MB
```

## Deployment

### Docker Compose
```bash
cd docker
docker-compose up -d
```

### Kubernetes
```bash
kubectl apply -f k8s/
```

### Cloud Platforms
- **AWS**: ECS, EKS, Lambda (with Quarkus AWS Lambda)
- **Azure**: AKS, Container Instances
- **GCP**: GKE, Cloud Run

## Architecture Details

### Game Server (WebSocket)
Real-time multiplayer using WebSocket:
- Player connections
- Game state synchronization
- Event broadcasting
- Packet handling

### Content Management System (REST)
HTTP API for content:
- CRUD operations
- Content validation
- Version management
- Admin interface

### Database Schema
PostgreSQL with Hibernate/Panache:
- Players
- Game state
- Inventory
- Achievements

## Performance

Typical performance metrics:

**JVM Mode:**
- Startup: ~1.5s
- Memory: ~200MB
- Throughput: ~10k req/s

**Native Mode:**
- Startup: ~0.08s
- Memory: ~50MB
- Throughput: ~12k req/s

## Monitoring

Built-in observability:
- **Health checks**: Liveness, readiness, startup
- **Metrics**: Micrometer + Prometheus
- **Tracing**: OpenTelemetry
- **Logging**: Structured JSON logs

## License

[Your License]
