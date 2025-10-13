# Template Override Examples

This guide shows how to customize each template type using **property overrides** and **trigger state overrides** without modifying the base template.

---

## Override Mechanism

### Minimal Spawn (No Overrides)

```protobuf
# Uses all template defaults
SpawnTemplateInstance {
  instance_id: "npc_001"
  template_id: "npc_merchant"
  transform { position { x: 10.0 y: 0.0 z: 5.0 } }
}
```

**Size:** ~112 bytes

### With Property Overrides

```protobuf
# Customize specific properties
SpawnTemplateInstance {
  instance_id: "npc_001"
  template_id: "npc_merchant"
  transform { position { x: 10.0 y: 0.0 z: 5.0 } }

  # Override default properties
  initial_properties {
    key: "greeting_text"
    value: "Welcome, traveler!"
  }
  initial_properties {
    key: "merchant_type"
    value: "weapons"
  }
}
```

**Size:** ~144 bytes (32 bytes extra for 2 overrides)

### With Trigger State Overrides

```protobuf
# Disable/modify specific triggers
SpawnTemplateInstance {
  instance_id: "npc_001"
  template_id: "npc_merchant"
  transform { position { x: 10.0 y: 0.0 z: 5.0 } }

  # Override trigger behavior
  trigger_overrides {
    trigger_id: "trigger_npc_wave"
    enabled: false  # Disable waving
  }
}
```

**Size:** ~128 bytes (16 bytes extra for trigger override)

---

## Template 1: npc_merchant

### Default Properties

```yaml
is_talking: false
greeting_shown: false
```

### Common Overrides

#### Example A: Grumpy Merchant (No Greeting)

```protobuf
SpawnTemplateInstance {
  instance_id: "npc_grumpy_001"
  template_id: "npc_merchant"
  transform { position { x: 10.0 y: 0.0 z: 5.0 } }

  # Disable wave trigger
  trigger_overrides {
    trigger_id: "trigger_npc_wave"
    enabled: false
  }

  # Custom greeting
  initial_properties {
    key: "greeting_text"
    value: "What do you want?"
  }
}
```

**Use case:** Unfriendly NPC

#### Example B: Distant Merchant (Wave from Farther)

```protobuf
SpawnTemplateInstance {
  instance_id: "npc_distant_001"
  template_id: "npc_merchant"
  transform { position { x: 10.0 y: 0.0 z: 5.0 } }

  # Increase wave distance
  initial_properties {
    key: "wave_distance"
    value: "10.0"  # Instead of 5.0
  }
}
```

**Note:** Requires template script to read this property and adjust trigger radius.

#### Example C: Important Quest Giver (Always Highlighted)

```protobuf
SpawnTemplateInstance {
  instance_id: "npc_quest_001"
  template_id: "npc_merchant"
  transform { position { x: 10.0 y: 0.0 z: 5.0 } }

  # Add quest marker property
  initial_properties {
    key: "has_quest"
    value: "true"
  }
  initial_properties {
    key: "quest_id"
    value: "main_quest_01"
  }
}
```

**Use case:** Quest giver with exclamation mark

---

## Template 2: door_standard

### Default Properties

```yaml
is_open: false
is_locked: false
```

### Common Overrides

#### Example A: Locked Door

```protobuf
SpawnTemplateInstance {
  instance_id: "door_locked_001"
  template_id: "door_standard"
  transform { position { x: 5.0 y: 0.0 z: 0.0 } }

  # Lock the door
  initial_properties {
    key: "is_locked"
    value: "true"
  }
  initial_properties {
    key: "required_key"
    value: "iron_key"
  }
}
```

**Use case:** Door requiring key

#### Example B: Already Open Door

```protobuf
SpawnTemplateInstance {
  instance_id: "door_open_001"
  template_id: "door_standard"
  transform { position { x: 5.0 y: 0.0 z: 0.0 } }

  # Start open
  initial_properties {
    key: "is_open"
    value: "true"
  }

  # Disable auto-close
  trigger_overrides {
    trigger_id: "trigger_door_autoclose"
    enabled: false
  }
}
```

**Use case:** Permanently open doorway

#### Example C: Quick-Close Door (2 seconds)

```protobuf
SpawnTemplateInstance {
  instance_id: "door_quick_001"
  template_id: "door_standard"
  transform { position { x: 5.0 y: 0.0 z: 0.0 } }

  # Faster auto-close
  initial_properties {
    key: "auto_close_delay"
    value: "2.0"  # Instead of 5.0
  }
}
```

**Use case:** Security door

---

## Template 3: treasure_chest

### Default Properties

```yaml
is_open: false
has_been_opened: false
loot_table: "treasure_common"
```

### Common Overrides

#### Example A: Rare Loot Chest

```protobuf
SpawnTemplateInstance {
  instance_id: "chest_rare_001"
  template_id: "treasure_chest"
  transform { position { x: 12.0 y: 0.0 z: 5.0 } }

  # Different loot table
  initial_properties {
    key: "loot_table"
    value: "treasure_rare"
  }

  # Brighter glow
  initial_properties {
    key: "glow_intensity"
    value: "1.5"  # Brighter than normal
  }
}
```

**Use case:** High-value chest

#### Example B: Already Opened Chest

```protobuf
SpawnTemplateInstance {
  instance_id: "chest_opened_001"
  template_id: "treasure_chest"
  transform { position { x: 12.0 y: 0.0 z: 5.0 } }

  # Already opened (for respawn scenarios)
  initial_properties {
    key: "is_open"
    value: "true"
  }
  initial_properties {
    key: "has_been_opened"
    value: "true"
  }

  # Disable all triggers
  trigger_overrides {
    trigger_id: "trigger_chest_hover"
    enabled: false
  }
  trigger_overrides {
    trigger_id: "trigger_chest_open"
    enabled: false
  }
}
```

**Use case:** Previously looted chest

#### Example C: Quest Chest (Specific Item)

```protobuf
SpawnTemplateInstance {
  instance_id: "chest_quest_001"
  template_id: "treasure_chest"
  transform { position { x: 12.0 y: 0.0 z: 5.0 } }

  # Quest-specific loot
  initial_properties {
    key: "loot_table"
    value: "quest_ancient_artifact"
  }
  initial_properties {
    key: "is_quest_chest"
    value: "true"
  }
  initial_properties {
    key: "quest_id"
    value: "ancient_ruins_quest"
  }
}
```

**Use case:** Story-driven loot

---

## Template 4: destructible_crate

### Default Properties

```yaml
health: 3.0
loot_table: "crate_common"
```

### Common Overrides

#### Example A: Weak Crate (1 Hit)

```protobuf
SpawnTemplateInstance {
  instance_id: "crate_weak_001"
  template_id: "destructible_crate"
  transform { position { x: 8.0 y: 0.0 z: 3.0 } }

  # Lower health
  initial_properties {
    key: "health"
    value: "1.0"
  }
}
```

**Use case:** Easy breakables

#### Example B: Reinforced Crate (10 Hits)

```protobuf
SpawnTemplateInstance {
  instance_id: "crate_reinforced_001"
  template_id: "destructible_crate"
  transform { position { x: 8.0 y: 0.0 z: 3.0 } }

  # High health
  initial_properties {
    key: "health"
    value: "10.0"
  }

  # Rare loot
  initial_properties {
    key: "loot_table"
    value: "crate_reinforced"
  }
}
```

**Use case:** High-value target

#### Example C: Empty Crate (No Loot)

```protobuf
SpawnTemplateInstance {
  instance_id: "crate_empty_001"
  template_id: "destructible_crate"
  transform { position { x: 8.0 y: 0.0 z: 3.0 } }

  # No loot
  initial_properties {
    key: "loot_table"
    value: ""
  }
}
```

**Use case:** Decoy/scenery

---

## Template 5: pressure_plate

### Default Properties

```yaml
is_pressed: false
linked_entity: ""
```

### Common Overrides

#### Example A: Pressure Plate → Door

```protobuf
# Door
SpawnTemplateInstance {
  instance_id: "door_001"
  template_id: "door_standard"
  transform { position { x: 5.0 y: 0.0 z: 0.0 } }
  initial_properties {
    key: "is_locked"
    value: "true"
  }
}

# Plate (linked to door)
SpawnTemplateInstance {
  instance_id: "plate_001"
  template_id: "pressure_plate"
  transform { position { x: 8.0 y: 0.0 z: 2.0 } }

  initial_properties {
    key: "linked_entity"
    value: "door_001"  # Links to door
  }
}
```

**Result:** Stepping on plate unlocks door

#### Example B: Heavy Pressure Plate

```protobuf
SpawnTemplateInstance {
  instance_id: "plate_heavy_001"
  template_id: "pressure_plate"
  transform { position { x: 8.0 y: 0.0 z: 2.0 } }

  # Requires heavy object
  initial_properties {
    key: "required_weight"
    value: "100.0"
  }
  initial_properties {
    key: "accepts_player"
    value: "false"  # Player too light
  }
}
```

**Use case:** Puzzle requiring box placement

#### Example C: Multi-Link Plate

```protobuf
SpawnTemplateInstance {
  instance_id: "plate_multi_001"
  template_id: "pressure_plate"
  transform { position { x: 8.0 y: 0.0 z: 2.0 } }

  # Multiple linked entities
  initial_properties {
    key: "linked_entities"
    value: "door_001,door_002,torch_001"  # Comma-separated
  }
}
```

**Use case:** Master switch

---

## Template 6: lever_switch

### Default Properties

```yaml
is_on: false
linked_entity: ""
```

### Common Overrides

#### Example A: Lever → Bridge

```protobuf
# Bridge
SpawnTemplateInstance {
  instance_id: "bridge_001"
  template_id: "moving_bridge"
  transform { position { x: 15.0 y: 0.0 z: 5.0 } }
}

# Lever (controls bridge)
SpawnTemplateInstance {
  instance_id: "lever_001"
  template_id: "lever_switch"
  transform { position { x: 10.0 y: 0.5 z: 3.0 } }

  initial_properties {
    key: "linked_entity"
    value: "bridge_001"
  }
}
```

**Result:** Lever extends/retracts bridge

#### Example B: Pre-Activated Lever

```protobuf
SpawnTemplateInstance {
  instance_id: "lever_on_001"
  template_id: "lever_switch"
  transform { position { x: 10.0 y: 0.5 z: 3.0 } }

  # Start in ON state
  initial_properties {
    key: "is_on"
    value: "true"
  }
}
```

**Use case:** Already activated state

#### Example C: Permanent Lever (No Toggle)

```protobuf
SpawnTemplateInstance {
  instance_id: "lever_permanent_001"
  template_id: "lever_switch"
  transform { position { x: 10.0 y: 0.5 z: 3.0 } }

  # One-time use
  initial_properties {
    key: "one_time_use"
    value: "true"
  }

  # Disable toggle trigger after first use
  trigger_overrides {
    trigger_id: "trigger_lever_toggle"
    trigger_count: 0  # Will disable after 1 use
  }
}
```

**Use case:** One-time switch

---

## Template 7: proximity_sensor

### Default Properties

```yaml
triggered: false
sensor_type: "proximity"
debug_visible: false
linked_entities: ""
```

### Common Overrides

#### Example A: Small Detection Zone (2m)

```protobuf
SpawnTemplateInstance {
  instance_id: "sensor_small_001"
  template_id: "proximity_sensor"
  transform {
    position { x: 20.0 y: 0.0 z: 10.0 }
    scale { x: 0.4 y: 0.4 z: 0.4 }  # 40% size = 2m radius
  }
}
```

**Use case:** Tight doorway sensor

#### Example B: Large Detection Zone (10m)

```protobuf
SpawnTemplateInstance {
  instance_id: "sensor_large_001"
  template_id: "proximity_sensor"
  transform {
    position { x: 20.0 y: 0.0 z: 10.0 }
    scale { x: 2.0 y: 2.0 z: 2.0 }  # 200% size = 10m radius
  }
}
```

**Use case:** Boss room entrance

#### Example C: Stealth Sensor (Silent)

```protobuf
SpawnTemplateInstance {
  instance_id: "sensor_stealth_001"
  template_id: "proximity_sensor"
  transform { position { x: 20.0 y: 0.0 z: 10.0 } }

  # No alarm sound
  initial_properties {
    key: "silent_mode"
    value: "true"
  }

  # Linked to security system
  initial_properties {
    key: "linked_entities"
    value: "security_system_001"
  }
}
```

**Use case:** Stealth gameplay

#### Example D: Debug Visible Sensor

```protobuf
SpawnTemplateInstance {
  instance_id: "sensor_debug_001"
  template_id: "proximity_sensor"
  transform { position { x: 20.0 y: 0.0 z: 10.0 } }

  # Show in debug mode
  initial_properties {
    key: "debug_visible"
    value: "true"
  }

  # Enable debug trigger
  trigger_overrides {
    trigger_id: "trigger_sensor_debug"
    enabled: true
  }
}
```

**Use case:** Development/testing

---

## Template 8: pickup_item

### Default Properties

```yaml
item_type: "coin"
item_value: "1"
```

### Common Overrides

#### Example A: Health Pickup

```protobuf
SpawnTemplateInstance {
  instance_id: "health_001"
  template_id: "pickup_item"
  transform {
    position { x: 15.0 y: 0.5 z: 8.0 }
    scale { x: 0.8 y: 0.8 z: 0.8 }
  }

  initial_properties {
    key: "item_type"
    value: "health"
  }
  initial_properties {
    key: "item_value"
    value: "25"  # Heal 25 HP
  }
}
```

**Use case:** Health potion

#### Example B: Gold Coin (High Value)

```protobuf
SpawnTemplateInstance {
  instance_id: "coin_gold_001"
  template_id: "pickup_item"
  transform {
    position { x: 15.0 y: 0.5 z: 8.0 }
    scale { x: 1.5 y: 1.5 z: 1.5 }  # Bigger = more valuable
  }

  initial_properties {
    key: "item_type"
    value: "coin"
  }
  initial_properties {
    key: "item_value"
    value: "100"
  }
  initial_properties {
    key: "rarity"
    value: "rare"
  }
}
```

**Use case:** Valuable collectible

#### Example C: Quest Item (Can't Be Missed)

```protobuf
SpawnTemplateInstance {
  instance_id: "key_quest_001"
  template_id: "pickup_item"
  transform { position { x: 15.0 y: 0.5 z: 8.0 } }

  initial_properties {
    key: "item_type"
    value: "quest_key"
  }
  initial_properties {
    key: "quest_id"
    value: "dungeon_escape"
  }
  initial_properties {
    key: "no_auto_pickup"
    value: "true"  # Requires explicit interaction
  }

  # Disable collision pickup
  trigger_overrides {
    trigger_id: "trigger_item_collect"
    enabled: false
  }
}
```

**Use case:** Important quest item

---

## Template 9: torch_light

### Default Properties

```yaml
is_lit: true
light_intensity: 1.0
light_color: "255,150,50"
```

### Common Overrides

#### Example A: Dim Torch (Dying Light)

```protobuf
SpawnTemplateInstance {
  instance_id: "torch_dim_001"
  template_id: "torch_light"
  transform { position { x: 5.0 y: 1.5 z: 2.0 } }

  initial_properties {
    key: "light_intensity"
    value: "0.5"  # 50% brightness
  }
}
```

**Use case:** Atmospheric lighting

#### Example B: Bright Torch (Main Light Source)

```protobuf
SpawnTemplateInstance {
  instance_id: "torch_bright_001"
  template_id: "torch_light"
  transform {
    position { x: 5.0 y: 1.5 z: 2.0 }
    scale { x: 1.5 y: 1.5 z: 1.5 }
  }

  initial_properties {
    key: "light_intensity"
    value: "2.0"  # 200% brightness
  }
  initial_properties {
    key: "light_color"
    value: "255,200,100"  # Warmer color
  }
}
```

**Use case:** Main lighting

#### Example C: Extinguished Torch (Can Be Lit)

```protobuf
SpawnTemplateInstance {
  instance_id: "torch_unlit_001"
  template_id: "torch_light"
  transform { position { x: 5.0 y: 1.5 z: 2.0 } }

  # Start unlit
  initial_properties {
    key: "is_lit"
    value: "false"
  }

  # Disable fire effects
  trigger_overrides {
    trigger_id: "trigger_torch_fire"
    enabled: false
  }
  trigger_overrides {
    trigger_id: "trigger_torch_sound"
    enabled: false
  }

  # Enable extinguish trigger (to light it)
  trigger_overrides {
    trigger_id: "trigger_torch_extinguish"
    enabled: true
  }
}
```

**Use case:** Puzzle torch

#### Example D: Blue Magical Torch

```protobuf
SpawnTemplateInstance {
  instance_id: "torch_magic_001"
  template_id: "torch_light"
  transform { position { x: 5.0 y: 1.5 z: 2.0 } }

  initial_properties {
    key: "light_color"
    value: "100,150,255"  # Blue
  }
  initial_properties {
    key: "is_magical"
    value: "true"
  }
}
```

**Use case:** Fantasy atmosphere

---

## Template 10: teleporter

### Default Properties

```yaml
is_active: true
destination: ""
destination_name: "Unknown"
is_one_way: false
```

### Common Overrides

#### Example A: Teleporter → Town

```protobuf
SpawnTemplateInstance {
  instance_id: "teleport_to_town"
  template_id: "teleporter"
  transform { position { x: 25.0 y: 0.0 z: 15.0 } }

  initial_properties {
    key: "destination"
    value: "spawn_town_center"  # Target location ID
  }
  initial_properties {
    key: "destination_name"
    value: "Town Center"
  }
}
```

**Use case:** Return to town

#### Example B: Inactive Teleporter (Quest-Locked)

```protobuf
SpawnTemplateInstance {
  instance_id: "teleport_locked_001"
  template_id: "teleporter"
  transform { position { x: 25.0 y: 0.0 z: 15.0 } }

  # Inactive until quest complete
  initial_properties {
    key: "is_active"
    value: "false"
  }
  initial_properties {
    key: "activation_quest"
    value: "ancient_ruins_complete"
  }

  # Disable all triggers
  trigger_overrides {
    trigger_id: "trigger_teleport_idle"
    enabled: false
  }
  trigger_overrides {
    trigger_id: "trigger_teleport_enter"
    enabled: false
  }
}
```

**Use case:** Progression-gated fast travel

#### Example C: One-Way Teleporter

```protobuf
SpawnTemplateInstance {
  instance_id: "teleport_oneway_001"
  template_id: "teleporter"
  transform { position { x: 25.0 y: 0.0 z: 15.0 } }

  initial_properties {
    key: "destination"
    value: "boss_arena"
  }
  initial_properties {
    key: "destination_name"
    value: "Boss Arena"
  }
  initial_properties {
    key: "is_one_way"
    value: "true"
  }
}
```

**Use case:** Point of no return

#### Example D: Red Teleporter (Enemy Base)

```protobuf
SpawnTemplateInstance {
  instance_id: "teleport_enemy_001"
  template_id: "teleporter"
  transform { position { x: 25.0 y: 0.0 z: 15.0 } }

  initial_properties {
    key: "destination"
    value: "enemy_fortress"
  }
  initial_properties {
    key: "destination_name"
    value: "Enemy Fortress"
  }
  initial_properties {
    key: "teleporter_color"
    value: "255,50,50"  # Red instead of blue
  }
}
```

**Use case:** Visual distinction

---

## Summary Table

| Template | Common Overrides | Size Impact |
|----------|------------------|-------------|
| **npc_merchant** | greeting_text, wave_distance, quest markers | +32-64 bytes |
| **door_standard** | is_locked, required_key, auto_close_delay | +32-48 bytes |
| **treasure_chest** | loot_table, glow_intensity, quest_id | +32-64 bytes |
| **destructible_crate** | health, loot_table | +16-32 bytes |
| **pressure_plate** | linked_entity, required_weight | +16-48 bytes |
| **lever_switch** | linked_entity, one_time_use, is_on | +16-48 bytes |
| **proximity_sensor** | scale (radius), silent_mode, linked_entities | +16-64 bytes |
| **pickup_item** | item_type, item_value, quest_id | +24-48 bytes |
| **torch_light** | is_lit, light_intensity, light_color | +24-48 bytes |
| **teleporter** | destination, destination_name, is_active | +48-96 bytes |

**Average overhead per override:** ~16-32 bytes

**Still much smaller than full template:** Template = 2-4 KB, Instance with overrides = 112-256 bytes (still 94-98% smaller!)

---

## Best Practices

### ✅ DO

**Use overrides for:**
- Dynamic values (linked entities, quest IDs)
- Instance-specific state (locked, opened, lit)
- Customization (loot tables, colors, intensities)
- Gameplay variations (health, damage, distance)

**Keep overrides minimal:**
```protobuf
# ✅ GOOD: Only override what's different
initial_properties {
  key: "loot_table"
  value: "treasure_rare"
}
```

**Use property naming conventions:**
```protobuf
# ✅ GOOD: Clear, specific names
key: "auto_close_delay"
key: "light_intensity"
key: "required_weight"
```

### ❌ DON'T

**Don't override everything:**
```protobuf
# ❌ BAD: Overriding too much (defeats purpose)
initial_properties { key: "property_1" value: "..." }
initial_properties { key: "property_2" value: "..." }
initial_properties { key: "property_3" value: "..." }
# ... 20 more overrides
```

**Don't duplicate template data:**
```protobuf
# ❌ BAD: This is already in template!
initial_properties {
  key: "animation_idle"
  value: "npc_idle_anim"
}
```

**Don't use generic names:**
```protobuf
# ❌ BAD: Unclear what "value" means
key: "value"
value: "5"

# ✅ GOOD: Specific name
key: "detection_radius"
value: "5.0"
```

---

## Delta Update Overrides

When properties change at runtime, server sends **delta** with only changed properties:

```protobuf
# Frame N: Door unlocked by player
TemplateInstanceDelta {
  instance_id: "door_001"

  # Only changed property
  changed_properties {
    key: "is_locked"
    value: "false"
  }
}
```

**Size:** ~40 bytes (only the change!)

---

## Override Summary

**Key Takeaways:**
- ✅ Overrides are **optional** (use template defaults)
- ✅ Overrides are **small** (16-96 bytes extra)
- ✅ Still **94-98% smaller** than full entity definition
- ✅ **Delta updates** send only changes (40 bytes)
- ✅ **Flexible** - customize any instance without changing template

**Your game server stays lean!** 🚀