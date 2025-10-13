# Game Data Hosting with Quarkus

This document explains how game data files are organized and served by the Siqiniq-001 Quarkus application.

## Directory Structure

All game data is now organized under `src/main/resources/META-INF/resources/game-data/`:

```
src/main/resources/META-INF/resources/game-data/
├── templates/          # Entity templates (NPCs, interactive objects, etc.)
│   ├── npc_merchant_template.pbtxt
│   ├── treasure_chest_template.pbtxt
│   ├── torch_light_template.pbtxt
│   └── ...
├── frames/            # Frame examples (spawn, delta, etc.)
│   ├── game_server_spawn_with_overrides.pbtxt
│   ├── game_server_minimal_spawn.pbtxt
│   └── game_server_minimal_delta.pbtxt
├── worlds/            # World/map definitions
│   └── sunny_plains.pbtxt
└── items/             # Item definitions
    └── weapons/
        └── swords.pbtxt
```

## Accessing Game Data

### Via REST API

The `GameDataResource` REST endpoint provides access to all game data files:

**Base URL:** `http://localhost:8080/game-data`

#### Examples:

1. **Get a template:**
   ```
   GET /game-data/templates/npc_merchant_template.pbtxt
   ```

2. **Get a world:**
   ```
   GET /game-data/worlds/sunny_plains.pbtxt
   ```

3. **Get an item (with subpath):**
   ```
   GET /game-data/items/weapons/swords.pbtxt
   ```

4. **Get a frame example:**
   ```
   GET /game-data/frames/game_server_spawn_with_overrides.pbtxt
   ```

5. **List files in a category:**
   ```
   GET /game-data/templates
   ```
   Returns: JSON array of available template files

### Via Static File Serving

Files can also be accessed directly as static resources:

```
http://localhost:8080/game-data/{category}/{path}
```

### Using curl:

```bash
# Fetch NPC merchant template
curl http://localhost:8080/game-data/templates/npc_merchant_template.pbtxt

# Fetch world definition
curl http://localhost:8080/game-data/worlds/sunny_plains.pbtxt

# Fetch item definition
curl http://localhost:8080/game-data/items/weapons/swords.pbtxt

# List all templates
curl http://localhost:8080/game-data/templates
```

## Adding New Game Data

To add new game data files:

1. **Place the file in the appropriate directory:**
   ```
   src/main/resources/META-INF/resources/game-data/{category}/
   ```

2. **For nested paths (like items/weapons):**
   ```
   src/main/resources/META-INF/resources/game-data/items/weapons/new_item.pbtxt
   ```

3. **Update the `GameDataResource.getKnownFiles()` method** (optional):
   - Add your new file to the static list for the category
   - This enables it to appear in the list endpoint

4. **Rebuild and restart:**
   ```bash
   mvn clean quarkus:dev
   ```

## File Formats

All game data files use Protocol Buffer Text Format (`.pbtxt`):

- **Human-readable** - Easy to edit and version control
- **Type-safe** - Validates against `.proto` definitions
- **Efficient** - Can be parsed into binary protobuf for performance

## API Documentation

Full API documentation is available via Swagger UI:

```
http://localhost:8080/swagger-ui
```

Or OpenAPI spec:
```
http://localhost:8080/openapi
```

## Configuration

Game data paths are configured in `application.yml`:

```yaml
siqiniq:
  data:
    base-path: "/META-INF/resources/game-data"
    templates-path: "${siqiniq.data.base-path}/templates"
    worlds-path: "${siqiniq.data.base-path}/worlds"
    items-path: "${siqiniq.data.base-path}/items"
    frames-path: "${siqiniq.data.base-path}/frames"
```

## Content Types

Files are served with appropriate content types:

- `.pbtxt` → `text/plain; charset=utf-8`
- `.proto` → `text/plain; charset=utf-8`
- `.json` → `application/json`
- `.md` → `text/markdown; charset=utf-8`

## CORS

CORS is enabled by default in development mode, allowing cross-origin requests from any domain.

For production, update `application.yml` to restrict origins:

```yaml
quarkus:
  http:
    cors:
      origins: "https://yourgame.com"
```

## Caching

Static resources are cached with a `max-age` of 1 hour (3600 seconds).

To disable caching during development:

```yaml
quarkus:
  http:
    static-resources:
      cache-control: "no-cache"
```

## Development Tips

1. **Hot Reload:** Files in `src/main/resources` are automatically reloaded in dev mode
2. **Validation:** Use `protoc` to validate `.pbtxt` files against `.proto` schemas
3. **Testing:** Use the Swagger UI to test endpoints interactively
4. **Monitoring:** Check logs for file access: `io.qimmiit.siqiniq.studio.resource.GameDataResource`

## Java API Usage

To load game data programmatically within your Quarkus application:

```java
@Inject
ResourceLoader resourceLoader;

// Load a template
InputStream stream = resourceLoader.getResourceAsStream(
    "/META-INF/resources/game-data/templates/npc_merchant_template.pbtxt"
);

// Or use the GameDataResource methods directly
@Inject
GameDataResource gameDataResource;

Response response = gameDataResource.getFile("templates", "npc_merchant_template.pbtxt");
```

## Migration Notes

The old structure:
- `content/` (top-level) → Moved to `src/main/resources/META-INF/resources/game-data/`
- `src/main/resources/*.pbtxt` → Moved to `src/main/resources/META-INF/resources/game-data/frames/`
- `src/main/resources/templates/` → Moved to `src/main/resources/META-INF/resources/game-data/templates/`

Original files remain in place as backups until verified working.
