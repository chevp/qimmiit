# Siqiniq-001 Quick Start Guide

Get your Quarkus game server running in minutes!

## 🚀 Super Quick Start (Development)

```bash
# 1. Start development server (with live reload)
./mvnw quarkus:dev

# 2. Open your browser
http://localhost:8080

# 3. Explore the API
http://localhost:8080/swagger-ui
```

That's it! The server is running with:
- ✅ WebSocket game server at `ws://localhost:8080/game/ws`
- ✅ REST API at `http://localhost:8080/api/*`
- ✅ Dev UI at `http://localhost:8080/q/dev/`
- ✅ Hot reload enabled (edit code and see changes instantly)

## 📋 Prerequisites

- **Java 21+** (Download from [Adoptium](https://adoptium.net/))
- **Maven 3.9+** (Or use included `./mvnw`)
- **Docker** (Optional, for PostgreSQL)

## 🎮 Development Workflow

### 1. Start Dev Mode
```bash
# Windows
scripts\run-dev.bat

# Linux/macOS
./mvnw quarkus:dev
```

### 2. Test WebSocket Connection

Create a simple HTML client:
```html
<!DOCTYPE html>
<html>
<body>
  <button onclick="connect()">Connect to Game</button>
  <script>
    function connect() {
      const ws = new WebSocket('ws://localhost:8080/game/ws');

      ws.onopen = () => {
        console.log('Connected to game server!');

        // Send connect packet (base64 encoded protobuf)
        ws.send(new Uint8Array([/* protobuf bytes */]));
      };

      ws.onmessage = (event) => {
        console.log('Received:', event.data);
      };
    }
  </script>
</body>
</html>
```

### 3. Test REST API
```bash
# List worlds
curl http://localhost:8080/api/worlds

# Get server status
curl http://localhost:8080/api/status

# List items
curl http://localhost:8080/api/items
```

### 4. Check Health
```bash
curl http://localhost:8080/q/health
```

### 5. View Metrics
```bash
curl http://localhost:8080/q/metrics
```

## 🔨 Building

### JVM Build (Fast)
```bash
# Windows
scripts\build.bat

# Linux/macOS
./mvnw clean package

# Run
java -jar target/quarkus-app/quarkus-run.jar
```

### Native Build (Ultra-fast startup)
```bash
# Requires GraalVM
scripts\build.bat --native

# Run (starts in ~0.08 seconds!)
./target/siqiniq-001-runner
```

## 🐳 Docker

### With Docker Compose (Recommended)
```bash
cd docker
docker-compose up -d

# View logs
docker-compose logs -f siqiniq-server

# Stop
docker-compose down
```

This starts:
- PostgreSQL database
- Siqiniq game server
- (Optional) Prometheus + Grafana for monitoring

### Manual Docker Build
```bash
# JVM image
docker build -f docker/Dockerfile.jvm -t siqiniq:jvm .
docker run -p 8080:8080 siqiniq:jvm

# Native image
docker build -f docker/Dockerfile.native -t siqiniq:native .
docker run -p 8080:8080 siqiniq:native
```

## 📡 API Endpoints

### Game Server (WebSocket)
- `ws://localhost:8080/game/ws` - Real-time game connection

### Content Management (REST)
- `GET /api/status` - Server status
- `GET /api/worlds` - List worlds
- `GET /api/worlds/{id}` - Get world
- `GET /api/items` - List items
- `GET /api/items/{id}` - Get item

### Health & Observability
- `GET /q/health` - Health check
- `GET /q/health/live` - Liveness probe
- `GET /q/health/ready` - Readiness probe
- `GET /q/metrics` - Prometheus metrics
- `GET /q/openapi` - OpenAPI spec
- `GET /swagger-ui` - Swagger UI

### Dev Mode Only
- `GET /q/dev/` - Dev UI dashboard

## 🎯 Next Steps

### 1. Add Your Game Logic
Edit files in `src/main/java/io/qimmiit/siqiniq/`:
- `server/gameplay/` - Game systems
- `server/network/` - Networking
- `cms/api/` - REST API endpoints

Changes are reflected instantly in dev mode!

### 2. Define Content
Edit proto files in `src/main/proto/`:
- `world.proto` - World schema
- `items.proto` - Items schema
- `game_packets.proto` - Network packets

Add content data in `content/`:
- `worlds/*.pbtxt` - World definitions
- `items/**/*.pbtxt` - Item definitions

### 3. Configure
Edit `src/main/resources/application.properties`:
```properties
siqiniq.game.max-players=100
siqiniq.game.tick-rate=60
```

### 4. Add Database Entities
Create JPA entities with Panache:
```java
@Entity
public class Player extends PanacheEntity {
    public String name;
    public int health;
}
```

### 5. Deploy

**Development:**
```bash
./mvnw quarkus:dev
```

**Production (JVM):**
```bash
./mvnw clean package
java -jar target/quarkus-app/quarkus-run.jar
```

**Production (Native):**
```bash
./mvnw clean package -Pnative
./target/siqiniq-001-runner
```

**Kubernetes:**
```bash
kubectl apply -f k8s/
```

## 💡 Tips & Tricks

### Live Coding
In dev mode, changes to:
- Java code → Automatically recompiled
- Resources → Automatically reloaded
- Proto files → Regenerated on next compile

### Testing
```bash
# Unit tests
./mvnw test

# Integration tests
./mvnw verify

# Specific test
./mvnw test -Dtest=GameServerTest
```

### Database
In dev mode, Quarkus automatically starts PostgreSQL via Dev Services (Docker required).

To use external database:
```properties
quarkus.datasource.jdbc.url=jdbc:postgresql://localhost:5432/siqiniq
```

### Performance Tuning
```properties
# Increase thread pool
quarkus.thread-pool.max-threads=200

# Tune WebSocket
quarkus.websocket.max-frame-size=1048576
```

## 📚 Learn More

- [Quarkus Documentation](https://quarkus.io/guides/)
- [Protocol Buffers Guide](https://protobuf.dev/getting-started/javatutorial/)
- [WebSocket Tutorial](https://quarkus.io/guides/websockets)
- [REST API Guide](https://quarkus.io/guides/rest)

## 🆘 Troubleshooting

**Port already in use:**
```bash
# Change port
./mvnw quarkus:dev -Dquarkus.http.port=8081
```

**Native build fails:**
Ensure you have GraalVM installed:
```bash
java -version  # Should show GraalVM
```

**Database connection error:**
Start PostgreSQL:
```bash
docker run -p 5432:5432 -e POSTGRES_PASSWORD=siqiniq postgres:16
```

## 🎊 You're Ready!

Start coding and building your game with Quarkus! 🚀
