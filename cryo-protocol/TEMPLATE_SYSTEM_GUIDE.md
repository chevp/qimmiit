# Template System Guide: CDN-Based Entity Definitions

## Problem

**Old Approach:** Game server sends full entity definitions (triggers, animations, sounds)

```
Game Server sends per entity:
  - Entity definition (mesh, collision)      ~1 KB
  - 4 triggers (full definitions)            ~3 KB
  - Animations (3 definitions)               ~500 bytes
  - Sounds (2 definitions)                   ~500 bytes
  - Properties                               ~200 bytes
  ────────────────────────────────────────
  Total per entity: ~5.2 KB

For 100 NPCs: ~520 KB per frame!
```

**Problems:**
- ❌ Game server overloaded with const data
- ❌ Massive bandwidth usage
- ❌ Can't cache (different instances each time)
- ❌ Slow spawn times

---

## Solution: Template System

**Separate const data (templates) from dynamic state (instances)**

```
Templates (const, from CDN):
  - Entity definition
  - Triggers
  - Animations
  - Sounds
  ────────────────────────────
  Loaded once from CDN
  Cached locally
  ~4 KB per template

Instances (dynamic, from game server):
  - Instance ID                 ~16 bytes
  - Template reference          ~16 bytes
  - Transform (position, etc)   ~48 bytes
  - Property overrides          ~32 bytes
  ────────────────────────────
  Total per instance: ~112 bytes!

For 100 NPCs: ~11 KB (97.9% smaller!)
```

---

## Architecture

### 1. Template Loading (from CDN)

```
Client starts
    ↓
Load template catalog from CDN
    ↓
Parse template references
    ↓
Preload common templates
    ↓
Cache templates locally
    ↓
Ready for instances!
```

### 2. Instance Spawning (from game server)

```
Game server decides to spawn NPC
    ↓
Send: { instance_id: "npc_001", template_id: "npc_merchant", pos: {10,0,5} }
    ↓ (112 bytes only!)
Client receives instance
    ↓
Look up "npc_merchant" template (from cache)
    ↓
Combine template + instance
    ↓
Spawn complete! (0 template data from game server!)
```

---

## Template Definition

### Template File (on CDN)
**URL:** `https://cdn.example.com/templates/npc_merchant.v1.pb`

```protobuf
EntityTemplate {
  template_id: "npc_merchant"
  template_version: "1.0.0"

  # Static entity definition
  entity_definition {
    mesh_id: "character_mesh_001"
    collision { capsule { radius: 0.4 height: 1.8 } }
  }

  # Static triggers (ifNear, onHover, onClick)
  triggers {
    trigger_id: "trigger_npc_wave"
    condition { if_near { distance: 5.0 } }
    on_enter { play_animation { name: "wave" } }
  }

  # Static animations
  animations {
    animation_name: "idle"
    animation_asset_id: "npc_idle_anim"
  }

  # Static sounds
  sounds {
    sound_id: "npc_greeting"
    sound_asset_id: "audio/npc/greeting.ogg"
  }

  # Default properties
  default_properties {
    key: "is_talking"
    value: "false"
  }
}
```

**Size:** ~4 KB (cached locally)
**Loaded:** Once from CDN
**Shared by:** All merchant NPCs

---

## Instance Spawning

### Game Server Sends (minimal!)

```protobuf
SpawnTemplateInstance {
  instance_id: "npc_merchant_001"
  template_id: "npc_merchant"  # Reference to cached template

  # ONLY dynamic state!
  transform {
    position { x: 10.0 y: 0.0 z: 5.0 }
    rotation { x: 0.0 y: 0.7071 z: 0.0 w: 0.7071 }
  }

  # Optional overrides (only if different from template)
  # (empty if using defaults)
}
```

**Size:** ~112 bytes (97.9% smaller than old approach!)

### Client Processing

```cpp
void Client::spawnInstance(const SpawnTemplateInstance& spawn) {
  // 1. Look up template (from cache)
  EntityTemplate* template = template_cache_.get(spawn.template_id());

  if (!template) {
    // Not cached yet, load from CDN
    loadTemplate(spawn.template_id());
    return;
  }

  // 2. Create entity from template
  Entity* entity = new Entity(spawn.instance_id());
  entity->setMesh(template->entity_definition().mesh_id());
  entity->setCollision(template->entity_definition().collision());

  // 3. Apply instance transform
  entity->setTransform(spawn.transform());

  // 4. Register triggers (from template)
  for (const auto& trigger : template->triggers()) {
    registerTrigger(entity, trigger);
  }

  // 5. Load animations (from template)
  for (const auto& anim : template->animations()) {
    entity->loadAnimation(anim);
  }

  // 6. Spawn complete!
  entities_[spawn.instance_id()] = entity;
}
```

---

## Delta Updates

### Game Server Sends (ONLY changes!)

```protobuf
TemplateInstanceDelta {
  instance_id: "npc_merchant_001"

  # ONLY if position changed
  transform {
    position { x: 10.5 y: 0.0 z: 5.2 }
  }

  # ONLY if properties changed
  changed_properties {
    key: "is_talking"
    value: "true"
  }

  # No need to send:
  # - Triggers (in template)
  # - Animations (in template)
  # - Sounds (in template)
  # - Unchanged properties
}
```

**Size:** ~40 bytes per entity (vs ~512 bytes old approach = 92% smaller!)

---

## Template Catalog

### Initial Connection (sent once)

```protobuf
TemplateCatalog {
  catalog_id: "game_catalog_v1"
  catalog_version: "1.0.0"

  # List of available templates
  templates {
    template_id: "npc_merchant"
    template_url: "https://cdn.example.com/templates/npc_merchant.v1.pb"
    size_bytes: 4096
    category: "npc"
  }

  templates {
    template_id: "door_standard"
    template_url: "https://cdn.example.com/templates/door_standard.v1.pb"
    size_bytes: 2048
    category: "props"
  }

  # ... more templates
}
```

**Client receives catalog → Preloads common templates → Ready!**

---

## Preloading Hints

### Server Hints Client About Upcoming Needs

```protobuf
PreloadHint {
  template_ids: "npc_merchant"
  template_ids: "npc_blacksmith"
  priority: HIGH
  reason: "entering_market_zone"
}
```

**Client receives hint → Loads templates in background → Seamless spawning!**

---

## Template Variants

### Same Template, Different Configs

```protobuf
# Base template: "npc_merchant"

# Variant 1: Friendly merchant (default)
TemplateVariant {
  variant_id: "npc_merchant_friendly"
  base_template_id: "npc_merchant"
  # Uses all defaults
}

# Variant 2: Grumpy merchant
TemplateVariant {
  variant_id: "npc_merchant_grumpy"
  base_template_id: "npc_merchant"

  property_overrides {
    key: "greeting_text"
    value: "What do you want?"
  }

  # Different wave animation
  # (rest inherited from base template)
}
```

**Game server spawns variants with just variant_id!**

---

## Bandwidth Comparison

### Spawning 100 NPCs

| Approach | Data Sent | Breakdown |
|----------|-----------|-----------|
| **Old (full definitions)** | **520 KB** | 100 × 5.2 KB per entity |
| **New (templates)** | **11 KB** | 100 × 112 bytes per instance |
| **Savings** | **509 KB (97.9%)** | Templates cached from CDN |

### Per-Frame Updates (100 entities)

| Approach | Data Sent | Breakdown |
|----------|-----------|-----------|
| **Old (full state)** | **51 KB** | 100 × 512 bytes per entity |
| **New (deltas only)** | **4 KB** | 100 × 40 bytes per change |
| **Savings** | **47 KB (92%)** | Only changed properties sent |

---

## Template Versioning

### Version Format

```protobuf
template_id: "npc_merchant"
template_version: "1.2.3"  # Semantic versioning
                           # Major.Minor.Patch
```

### Version Changes

```protobuf
TemplateVersion {
  template_id: "npc_merchant"
  version: "2.0.0"

  changes {
    type: MODIFIED
    description: "Changed wave animation to new asset"
  }

  breaking_change: true
  migration_script: "update_merchant_instances_v2.lua"
}
```

### Client Handles Versioning

```cpp
void Client::loadTemplate(const std::string& template_id) {
  // Check cache
  auto cached = template_cache_.get(template_id);

  if (cached && cached->version() == latest_version) {
    return;  // Use cached version
  }

  // Load new version from CDN
  std::string url = catalog_.getTemplateURL(template_id);
  downloadTemplate(url);
}
```

---

## Caching Strategy

### HTTP Caching Headers

```http
GET /templates/npc_merchant.v1.pb
Host: cdn.example.com

HTTP/1.1 200 OK
Cache-Control: public, max-age=31536000, immutable
ETag: "v1.0.0-abc123"
Content-Type: application/octet-stream
Content-Length: 4096

[template binary data]
```

**Benefits:**
- Templates cached for 1 year
- Browser/CDN handles caching
- Version in URL = cache-friendly
- Immutable = perfect caching

### Local Storage Caching

```javascript
// Client-side cache
class TemplateCache {
  async get(templateId) {
    // 1. Check memory cache
    if (this.memoryCache.has(templateId)) {
      return this.memoryCache.get(templateId);
    }

    // 2. Check IndexedDB cache
    const cached = await this.db.get(templateId);
    if (cached) {
      this.memoryCache.set(templateId, cached);
      return cached;
    }

    // 3. Fetch from CDN
    return await this.loadFromCDN(templateId);
  }

  async loadFromCDN(templateId) {
    const url = this.catalog.getTemplateURL(templateId);
    const response = await fetch(url);
    const template = await response.arrayBuffer();

    // Cache in IndexedDB
    await this.db.put(templateId, template);

    // Cache in memory
    this.memoryCache.set(templateId, template);

    return template;
  }
}
```

---

## Implementation Example

### Server-Side (Game Server)

```cpp
class GameServer {
public:
  void spawnNPC(const std::string& template_id, const Vector3& position) {
    std::string instance_id = generateInstanceID();

    // Create minimal spawn message
    SpawnTemplateInstance spawn;
    spawn.set_instance_id(instance_id);
    spawn.set_template_id(template_id);  // Just reference!
    spawn.mutable_transform()->mutable_position()->CopyFrom(position);

    // Send to clients (~112 bytes!)
    broadcastToClients(spawn);

    // Store instance state (minimal!)
    instances_[instance_id] = {
      .template_id = template_id,
      .position = position,
      .properties = {}  // Empty if using template defaults
    };
  }

  void updateNPC(const std::string& instance_id, const Vector3& new_position) {
    // Create minimal delta
    TemplateInstanceDelta delta;
    delta.set_instance_id(instance_id);
    delta.mutable_transform()->mutable_position()->CopyFrom(new_position);

    // Send to clients (~40 bytes!)
    broadcastToClients(delta);
  }
};
```

### Client-Side

```cpp
class GameClient {
private:
  TemplateCache template_cache_;
  std::map<std::string, Entity*> entities_;

public:
  void handleSpawn(const SpawnTemplateInstance& spawn) {
    // Look up template
    auto* template_def = template_cache_.get(spawn.template_id());

    if (!template_def) {
      // Load from CDN asynchronously
      template_cache_.loadAsync(spawn.template_id(), [this, spawn]() {
        handleSpawn(spawn);  // Retry after loaded
      });
      return;
    }

    // Create entity from template
    Entity* entity = createEntityFromTemplate(template_def);
    entity->setTransform(spawn.transform());

    // Apply property overrides
    for (const auto& [key, value] : spawn.initial_properties()) {
      entity->setProperty(key, value);
    }

    entities_[spawn.instance_id()] = entity;
  }

  void handleDelta(const TemplateInstanceDelta& delta) {
    auto* entity = entities_[delta.instance_id()];

    if (delta.has_transform()) {
      entity->setTransform(delta.transform());
    }

    for (const auto& [key, value] : delta.changed_properties()) {
      entity->setProperty(key, value);
    }
  }
};
```

---

## Best Practices

### ✅ DO

**Use templates for:**
- NPCs with same behavior
- Doors, chests, switches
- Enemies of same type
- Decorative objects
- Trigger volumes

**Version templates properly:**
```
npc_merchant.v1.0.0.pb  ✅
npc_merchant.v1.0.1.pb  ✅ (bug fix)
npc_merchant.v2.0.0.pb  ✅ (breaking change)
```

**Cache aggressively:**
```
Cache-Control: public, max-age=31536000, immutable
```

**Use preload hints:**
```protobuf
PreloadHint {
  template_ids: "boss_dragon"
  priority: CRITICAL
  reason: "approaching_boss_room"
}
```

### ❌ DON'T

**Don't put dynamic data in templates:**
```protobuf
# ❌ BAD: Position is dynamic!
entity_definition {
  position { x: 10.0 y: 0.0 z: 5.0 }
}

# ✅ GOOD: Position in instance!
SpawnTemplateInstance {
  transform {
    position { x: 10.0 y: 0.0 z: 5.0 }
  }
}
```

**Don't version unnecessarily:**
```
npc_merchant.v1.0.0.pb  ✅
npc_merchant.v1.0.1.pb  ❌ (no changes? why new version?)
```

**Don't send templates from game server:**
```cpp
// ❌ BAD: Game server sends template
server.send(entity_template);  // Heavy!

// ✅ GOOD: Game server sends reference
server.send({ template_id: "npc_merchant" });  // Light!
```

---

## Summary

### Template System Benefits

| Benefit | Impact |
|---------|--------|
| **Game server load** | 97.9% reduction in data sent |
| **Bandwidth** | 92%+ reduction in frame updates |
| **Spawn speed** | Instant (template cached) |
| **Scalability** | 1000+ entities no problem |
| **Updates** | Template updates via CDN, not game patch |
| **Caching** | HTTP cache = perfect caching |

### Architecture

```
┌─────────────────┐         ┌─────────────────┐
│   CDN Server    │         │  Game Server    │
│  (Templates)    │         │   (Instances)   │
└────────┬────────┘         └────────┬────────┘
         │                           │
         │ Load template (once)      │ Spawn instance
         │ ~4 KB                      │ ~112 bytes
         │                           │
         ├───────────────────────────┤
         │                           │
         ▼                           ▼
    ┌─────────────────────────────────┐
    │          Client                 │
    │                                 │
    │  Template Cache (CDN data)      │
    │  + Instance State (game data)   │
    │  = Complete Entity              │
    └─────────────────────────────────┘
```

### Files Created

- [`proto/ecs/cryo_templates.proto`](proto/ecs/cryo_templates.proto) - Template protocol
- [`examples/templates/npc_merchant_template.pbtxt`](examples/templates/npc_merchant_template.pbtxt) - Template example
- [`examples/game_server_minimal_spawn.pbtxt`](examples/game_server_minimal_spawn.pbtxt) - Minimal spawning
- [`examples/game_server_minimal_delta.pbtxt`](examples/game_server_minimal_delta.pbtxt) - Minimal deltas

---

**Your game server is now unburdened from const data!** 🚀

**97.9% bandwidth reduction = scalable to 1000+ entities!**