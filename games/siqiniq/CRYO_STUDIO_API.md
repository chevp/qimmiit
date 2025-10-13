# Cryo Studio API Documentation

This document describes the REST and gRPC APIs for interacting with Cryo Protocol templates and scenes from studio development tools.

## Overview

The Siqiniq-001 game server provides two types of APIs for studio integration:

1. **REST API (HTTP 1.1)** - For traditional HTTP clients and web-based tools
2. **gRPC API** - For high-performance, type-safe communication

Both APIs provide identical functionality through different protocols.

## Base URLs

- **REST API**: `http://localhost:8080/api/studio`
- **gRPC API**: `localhost:9000`
- **OpenAPI/Swagger UI**: `http://localhost:8080/q/swagger-ui`

## Template Management API

### REST Endpoints

#### Create Template
```http
POST /api/studio/templates
Content-Type: application/json

{
  "name": "player_character",
  "description": "Base player character template",
  "type": "ENTITY",
  "content": "id: \"player_001\"\ncomponents { ... }",
  "created_by": "studio",
  "tags": {
    "category": "character",
    "game": "rpg"
  }
}
```

**Response**: `201 Created`
```json
{
  "metadata": {
    "id": "abc-123",
    "name": "player_character",
    "type": "ENTITY",
    "created_at": "2025-01-15T10:30:00Z",
    ...
  },
  "content": "..."
}
```

#### Get Template
```http
GET /api/studio/templates/{id}?includeContent=true
```

**Response**: `200 OK`

#### List Templates
```http
GET /api/studio/templates?page=0&pageSize=20&search=player
```

**Response**: `200 OK`
```json
{
  "templates": [...],
  "total_count": 42,
  "page": 0,
  "page_size": 20
}
```

#### List Templates by Type
```http
GET /api/studio/templates/by-type/ENTITY?page=0&pageSize=20
```

#### Update Template
```http
PUT /api/studio/templates/{id}
Content-Type: application/json

{
  "name": "updated_name",
  "content": "...",
  "updated_by": "studio"
}
```

**Response**: `200 OK`

#### Delete Template
```http
DELETE /api/studio/templates/{id}
```

**Response**: `204 No Content`

#### Load Template as Runtime
```http
POST /api/studio/templates/{id}/load
Content-Type: application/json

{
  "overrides": {
    "position.x": "10.0",
    "position.y": "5.0"
  }
}
```

**Response**: `200 OK`
```json
{
  "runtime_id": "runtime-xyz",
  "success": true,
  "message": "Template loaded successfully"
}
```

#### Validate Template
```http
POST /api/studio/templates/validate
Content-Type: application/json

{
  "content": "id: \"test\"\ncomponents { ... }",
  "type": "ENTITY"
}
```

**Response**: `200 OK`
```json
{
  "valid": true,
  "errors": [],
  "warnings": []
}
```

### Template Types

- `ENTITY` - Entity templates
- `SCENE` - Scene templates
- `AGENT` - Agent/NPC templates
- `ITEM` - Item templates
- `CHARACTER` - Character templates
- `ENVIRONMENT` - Environment templates
- `EFFECT` - Visual effect templates
- `UI` - UI element templates
- `MATERIAL` - Material templates
- `SHADER` - Shader templates

## Scene Authoring API

### REST Endpoints

#### Create Scene
```http
POST /api/studio/scenes
Content-Type: application/json

{
  "name": "main_world",
  "description": "Main game world scene",
  "project_id": "project-001",
  "created_by": "studio",
  "tags": ["world", "main"],
  "initial_content": "..."
}
```

**Response**: `201 Created`

#### Get Scene
```http
GET /api/studio/scenes/{id}?format=PBTXT&includeEntities=true
```

Formats: `PBTXT`, `JSON`, `BINARY`, `YAML`

#### Update Scene
```http
PUT /api/studio/scenes/{id}
Content-Type: application/json

{
  "name": "updated_world",
  "description": "Updated description",
  "status": "PUBLISHED",
  "tags": ["world", "main", "v1.0"]
}
```

#### Delete Scene
```http
DELETE /api/studio/scenes/{id}
```

#### List Scenes
```http
GET /api/studio/scenes?project_id=project-001&page=0&pageSize=20&search=world&status=DRAFT
```

### Entity Management within Scene

#### Add Entity to Scene
```http
POST /api/studio/scenes/{sceneId}/entities
Content-Type: application/json

{
  "entity_content": "id: \"entity_001\"\ncomponents { ... }",
  "parent_id": "parent_entity_id"  // Optional
}
```

**Response**: `201 Created`

#### Update Entity
```http
PUT /api/studio/scenes/{sceneId}/entities/{entityId}
Content-Type: application/json

{
  "entity_content": "..."
}
```

#### Remove Entity
```http
DELETE /api/studio/scenes/{sceneId}/entities/{entityId}?removeChildren=false
```

#### List Entities
```http
GET /api/studio/scenes/{sceneId}/entities?parentId=root&page=0&pageSize=50
```

### Hierarchy Management

#### Get Scene Hierarchy
```http
GET /api/studio/scenes/{sceneId}/hierarchy?rootEntityId=root&maxDepth=0
```

**Response**: `200 OK`
```json
{
  "nodes": [
    {
      "entity_id": "root",
      "name": "Root",
      "type": "Container",
      "enabled": true,
      "children": [
        {
          "entity_id": "child_001",
          "name": "Player",
          "type": "Character",
          "enabled": true,
          "children": []
        }
      ]
    }
  ]
}
```

### Export/Import

#### Export Scene
```http
POST /api/studio/scenes/{sceneId}/export
Content-Type: application/json

{
  "format": "PBTXT",
  "options": {
    "include_assets": true,
    "include_metadata": true,
    "pretty_print": true
  }
}
```

**Response**: `200 OK` (Binary file download)
```
Content-Type: application/octet-stream
Content-Disposition: attachment; filename="scene_{id}.pbtxt"
X-Checksum: sha256-hash
```

#### Import Scene
```http
POST /api/studio/scenes/import?name=imported_scene&projectId=proj-001&format=PBTXT&createdBy=studio
Content-Type: application/octet-stream

[Binary scene data]
```

**Response**: `201 Created`

## gRPC API

### Service Definitions

Two gRPC services are available:

1. **CryoTemplateService** - Template CRUD operations
2. **CryoSceneAuthoringService** - Full scene authoring

### Proto Files

- `cryo-services/proto/cryo_template_service.proto`
- `cryo-services/proto/cryo_scene_authoring_service.proto`

### Example gRPC Client (Java)

```java
import cryo.services.CryoTemplateServiceGrpc;
import cryo.services.CryoTemplateServiceOuterClass.*;

// Create channel
ManagedChannel channel = ManagedChannelBuilder
    .forAddress("localhost", 9000)
    .usePlaintext()
    .build();

// Create stub
CryoTemplateServiceGrpc.CryoTemplateServiceBlockingStub stub =
    CryoTemplateServiceGrpc.newBlockingStub(channel);

// Create template
CreateTemplateRequest request = CreateTemplateRequest.newBuilder()
    .setName("player_character")
    .setDescription("Base player character")
    .setType(TemplateType.ENTITY)
    .setContent("id: \"player_001\"")
    .setCreatedBy("studio")
    .build();

TemplateResponse response = stub.createTemplate(request);
System.out.println("Created template: " + response.getMetadata().getId());

// Close channel
channel.shutdown();
```

## Scene Status

- `DRAFT` - Scene is in development
- `PUBLISHED` - Scene is ready for production
- `ARCHIVED` - Scene is archived

## Error Handling

### HTTP Status Codes

- `200 OK` - Successful GET/PUT request
- `201 Created` - Successful POST request
- `204 No Content` - Successful DELETE request
- `400 Bad Request` - Invalid request data
- `404 Not Found` - Resource not found
- `500 Internal Server Error` - Server error

### Error Response Format

```json
{
  "error": "Template not found: abc-123"
}
```

## Development Workflow

### Typical Studio Integration

1. **List available templates** by type
2. **Get template** content for editing
3. **Update template** with changes
4. **Validate template** before saving
5. **Load template** as runtime instance for testing
6. **Create scenes** from templates
7. **Export scenes** for deployment

### Real-time Collaboration (Coming Soon)

The gRPC API includes streaming endpoints for real-time collaboration:

- `StreamSceneUpdates` - Stream live scene updates
- `ApplySceneUpdate` - Apply updates to shared scene

## Building and Running

### Build Proto Files

```bash
cd games/siqiniq-001
mvn clean compile
```

This will generate Java classes from proto files in:
- `target/generated-sources/protobuf/java`
- `target/generated-sources/protobuf/grpc-java`

### Run Server

```bash
mvn quarkus:dev
```

Server will start on:
- HTTP: `localhost:8080`
- gRPC: `localhost:9000`

### Access Swagger UI

Open browser to: `http://localhost:8080/q/swagger-ui`

## Configuration

Edit `src/main/resources/application.yml`:

```yaml
quarkus:
  http:
    port: 8080
  grpc:
    server:
      port: 9000
```

## Security (Production)

For production deployments, add:

1. **Authentication** - OAuth2/JWT tokens
2. **Authorization** - Role-based access control
3. **Rate Limiting** - API rate limits
4. **TLS/SSL** - Encrypted connections
5. **API Keys** - API key management

## See Also

- [Cryo Protocol Documentation](../../../cryo-protocol/README.md)
- [Service Proto Files](../../../cryo-services/proto/)
- [Siqiniq-001 Architecture](ARCHITECTURE.md)
- [gRPC Implementation Guide](GRPC_IMPLEMENTATION.md)
