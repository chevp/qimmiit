# State Triggers Guide: Reactive World Interactions

## Overview

The **State Trigger System** enables **client-side reactive behaviors** based on state conditions. This eliminates network round-trips for common interactions, providing **instant visual feedback** (0ms response time).

### Key Concepts

```
State Condition → Client Evaluates → Immediate Action
      ↓                                      ↓
  ifNear, onHover                   Animation, Sound, Effect
   onStand, etc.                     (No server delay!)
```

### Core Features

- ✅ **0ms response time** for visual feedback
- ✅ **Client-side evaluation** of conditions
- ✅ **Immediate execution** of actions
- ✅ **Asynchronous server notification** (when needed)
- ✅ **Declarative syntax** (server defines rules)

---

## Condition Types

### 1. Distance-Based Conditions

#### `ifNear` - Trigger when entity approaches

```protobuf
condition {
  if_near {
    target_entity_id: "player"
    distance: 5.0
    mode: HORIZONTAL_ONLY
    require_in_front: true
    field_of_view: 120.0
  }
}
```

**Use cases:**
- NPC waves when player approaches
- Enemy enters alert state
- Environmental sounds activate
- Tutorial hints appear

**Example: NPC Greeting**
```
Player walks within 5m → NPC waves → Greeting sound plays
                ↓ 0ms
        All client-side!
```

#### `ifFar` - Trigger when entity moves away

```protobuf
condition {
  if_far {
    target_entity_id: "player"
    distance: 10.0
  }
}
```

**Use cases:**
- Despawn distant objects
- NPC returns to idle
- Stop looping sounds
- Hide UI elements

---

### 2. Mouse/Pointer Conditions

#### `onHover` - Trigger on mouse hover

```protobuf
condition {
  on_hover {
    require_raycast_hit: true  # Must be visible
    hover_delay: 0.1           # 100ms delay
  }
}
```

**Use cases:**
- Button color change
- Object highlight
- Tooltip display
- Name label show

**Example: Interactive Object**
```
Mouse hovers object → Highlight + Tooltip
            ↓ 0ms
    Client-side, instant!
```

#### `onClick` - Trigger on mouse click

```protobuf
condition {
  on_click {
    button: 0          # Left mouse button
    require_hover: true
  }
}
```

**Use cases:**
- Open doors
- Pick up items
- Start NPC dialogue
- Activate switches

---

### 3. Spatial Conditions

#### `onStand` - Trigger when standing on surface

```protobuf
condition {
  on_stand {
    surface_entity_id: "pressure_plate_001"
    surface_tolerance: 0.2
    require_grounded: true
  }
}
```

**Use cases:**
- Pressure plates
- Moving platforms
- Floor triggers
- Step-activated switches

**Example: Pressure Plate**
```
Player stands on plate → Plate depresses → Door opens
                  ↓ 0ms
          Visual feedback instant!
```

#### `onCollide` - Trigger on collision

```protobuf
condition {
  on_collide {
    other_entity_id: "player"
    type: ENTER  # ENTER, STAY, or EXIT
  }
}
```

**Use cases:**
- Proximity sensors
- Damage zones
- Pickup items
- Level boundaries

#### `inZone` - Trigger when in zone

```protobuf
condition {
  in_zone {
    sphere {
      center { x: 10.0 y: 0.0 z: 5.0 }
      radius: 3.0
    }
    target_entity_id: "player"
  }
}
```

**Use cases:**
- Area-of-effect spells
- Music transition zones
- Weather regions
- Safe zones

---

### 4. State-Based Conditions

#### `onStateChange` - Trigger when property changes

```protobuf
condition {
  on_state_change {
    property_name: "is_open"
    bool_value: true
  }
}
```

**Use cases:**
- Chain reactions
- State-dependent behaviors
- UI updates
- Animation transitions

**Example: Door Chain**
```
Door 1 opens → Triggers Door 2 → Door 2 opens → Triggers Door 3
                      ↓
              All cascades instantly!
```

#### `hasProperty` - Trigger when property matches condition

```protobuf
condition {
  has_property {
    property_name: "health"
    operator: LESS_THAN
    float_value: 0.3  # 30% health
  }
}
```

**Use cases:**
- Low health effects
- State transitions
- Conditional behaviors
- Threshold alerts

---

### 5. Visibility Conditions

#### `isVisible` - Trigger when visible to observer

```protobuf
condition {
  is_visible {
    from_entity_id: "player"
    require_unoccluded: true
  }
}
```

**Use cases:**
- Enemy detection
- Stealth mechanics
- LOD transitions
- Culling optimization

#### `inFrustum` - Trigger when in camera view

```protobuf
condition {
  in_frustum {
    camera_id: "main_camera"
  }
}
```

**Use cases:**
- Activate visible AI
- Start animations
- Audio cues
- Performance optimization

---

### 6. Time-Based Conditions

#### `afterTime` - Trigger after delay

```protobuf
condition {
  after_time {
    delay_seconds: 5.0
    reset_on_re_enable: true
  }
}
```

**Use cases:**
- Auto-close doors
- Timed events
- Delayed reactions
- Cooldown timers

#### `everyInterval` - Trigger repeatedly

```protobuf
condition {
  every_interval {
    interval_seconds: 2.0
    align_to_world_time: false
  }
}
```

**Use cases:**
- Periodic animations
- Spawn timers
- Heartbeat effects
- Regular updates

---

### 7. Composite Conditions

#### `andCondition` - All must be true

```protobuf
condition {
  and_condition {
    conditions {
      on_hover { }
    }
    conditions {
      if_near {
        target_entity_id: "player"
        distance: 3.0
      }
    }
  }
}
```

**Example: Door Interaction**
```
Must hover AND be near → Highlight door → Click opens
```

#### `orCondition` - Any must be true

```protobuf
condition {
  or_condition {
    conditions {
      on_click { button: 0 }  # Left click
    }
    conditions {
      on_click { button: 2 }  # OR right click
    }
  }
}
```

#### `notCondition` - Invert condition

```protobuf
condition {
  not_condition {
    condition {
      has_property {
        property_name: "is_locked"
        bool_value: true
      }
    }
  }
}
```

**Meaning:** "If NOT locked, allow interaction"

---

## Action Types

### Animation Actions

#### `playAnimation` - Play animation

```protobuf
on_enter {
  play_animation {
    animation_name: "wave_greeting"
    speed: 1.0
    loop: false
    blend_time: 0.3
  }
  client_side: true
}
```

**Available options:**
- `speed`: Playback speed multiplier (default: 1.0)
- `loop`: Loop animation (default: false)
- `blend_time`: Blend duration in seconds (default: 0.0)
- `start_time`: Start at specific time (default: 0.0)

#### `stopAnimation` - Stop animation

```protobuf
on_exit {
  stop_animation {
    animation_name: "wave_greeting"
    blend_out_time: 0.5
  }
  client_side: true
}
```

---

### Audio Actions

#### `playSound` - Play sound effect

```protobuf
on_enter {
  play_sound {
    sound_id: "door_creak"
    volume: 0.6
    pitch: 1.0
    loop: false
    spatial_mode: POSITIONAL_3D
    position { x: 5.0 y: 1.0 z: 0.0 }
  }
  client_side: true
}
```

**Spatial modes:**
- `POSITIONAL_3D`: 3D positioned sound (distance falloff)
- `AMBIENT`: Non-positioned background sound
- `UI`: UI sound (full volume, no falloff)

#### `stopSound` - Stop sound

```protobuf
on_exit {
  stop_sound {
    sound_id: "ambient_loop"
    fade_out_time: 2.0
  }
  client_side: true
}
```

---

### Visual Effect Actions

#### `playEffect` - Spawn particle effect

```protobuf
on_enter {
  play_effect {
    effect_id: "sparkle_burst"
    position { x: 12.0 y: 0.8 z: 5.0 }
    scale: 1.5
    duration: 2.0
  }
  client_side: true
}
```

#### `setColor` - Change material color

```protobuf
on_enter {
  set_color {
    color { r: 1.2 g: 1.2 b: 1.0 a: 1.0 }
    transition_time: 0.2
    property: "emissive"
  }
  client_side: true
}
```

**Properties:**
- `albedo`: Base color
- `emissive`: Glow color
- `specular`: Specular highlight color

#### `setVisibility` - Show/hide entity

```protobuf
on_enter {
  set_visibility {
    visible: false
    fade_time: 0.5
  }
  client_side: true
}
```

---

### Transform Actions

#### `moveTo` - Move entity

```protobuf
on_enter {
  move_to {
    target_position { x: 0.0 y: -0.05 z: 0.0 }
    relative: true
  }
  duration: 0.15
  easing: "ease_out_cubic"
  client_side: true
}
```

#### `rotateTo` - Rotate entity

```protobuf
on_enter {
  rotate_to {
    target_rotation { x: 0.0 y: 0.707 z: 0.0 w: 0.707 }
    relative: false
  }
  duration: 0.5
  client_side: true
}
```

#### `scaleTo` - Scale entity

```protobuf
on_enter {
  scale_to {
    target_scale { x: 1.2 y: 1.2 z: 1.2 }
    relative: false
  }
  duration: 0.3
  easing: "ease_in_out_quad"
  client_side: true
}
```

---

### State Actions

#### `setProperty` - Set entity property

```protobuf
on_enter {
  set_property {
    property_name: "is_open"
    bool_value: true
  }
  client_side: true
}
```

**Supported types:**
- `float_value`
- `int_value`
- `bool_value`
- `string_value`

#### `incrementProperty` - Increment numeric property

```protobuf
on_enter {
  increment_property {
    property_name: "use_count"
    increment: 1.0
    min_value: 0.0
    max_value: 100.0
  }
  client_side: true
}
```

---

### Entity Actions

#### `spawnEntity` - Spawn new entity

```protobuf
on_enter {
  spawn_entity {
    prefab_id: "loot_gold"
    spawn_position { x: 12.0 y: 1.0 z: 5.0 }
    spawn_rotation { x: 0.0 y: 0.0 z: 0.0 w: 1.0 }
    spawn_entity_id: "loot_instance_001"
  }
  client_side: false  # Server validation needed
}
```

#### `destroyEntity` - Destroy entity

```protobuf
on_enter {
  destroy_entity {
    entity_id: "temporary_effect"
    delay: 2.0
  }
  client_side: true
}
```

---

### UI Actions

#### `showUI` - Show UI element

```protobuf
on_enter {
  show_ui {
    ui_element_id: "ui_npc_name_merchant"
    fade_in_time: 0.2
  }
  client_side: true
}
```

#### `hideUI` - Hide UI element

```protobuf
on_exit {
  hide_ui {
    ui_element_id: "ui_npc_name_merchant"
    fade_out_time: 0.2
  }
  client_side: true
}
```

---

### Camera Actions

#### `shakeCamera` - Camera shake effect

```protobuf
on_enter {
  shake_camera {
    intensity: 0.5
    frequency: 20.0
  }
  duration: 0.3
  client_side: true
}
```

#### `focusCamera` - Focus camera on entity

```protobuf
on_enter {
  focus_camera {
    target_entity_id: "cutscene_actor"
    transition_time: 2.0
  }
  client_side: false
}
```

---

## Action Lifecycle

### Trigger Phases

```
Condition becomes TRUE:
  → on_enter actions execute (once)
  → while_active actions execute (every frame)

Condition remains TRUE:
  → while_active continues

Condition becomes FALSE:
  → while_active stops
  → on_exit actions execute (once)
```

### Example: Button Press

```protobuf
StateTrigger {
  condition { on_stand { } }

  # Phase 1: Button pressed
  on_enter {
    move_to { y: -0.05 }  # Depress
    play_sound { sound_id: "button_press" }
  }

  # Phase 2: While pressed
  while_active {
    set_color { color { r: 0.0 g: 1.0 b: 0.0 } }  # Green
  }

  # Phase 3: Button released
  on_exit {
    move_to { y: 0.05 }  # Rise back up
    play_sound { sound_id: "button_release" }
  }
}
```

---

## Trigger Options

### Basic Options

```protobuf
options {
  enabled: true           # Can be disabled dynamically
  one_shot: true          # Trigger only once
  cooldown: 2.0           # 2 second cooldown between triggers
  priority: 10            # Higher priority = evaluated first
  retrigger_on_exit: true # Can trigger again after exit
  min_retrigger_delay: 0.5  # Minimum time between retrigers
}
```

### Debug Options

```protobuf
options {
  debug_visualize: true      # Show debug visualization
  debug_color: "#FF0000"     # Red debug color
}
```

---

## Complete Examples

### Example 1: NPC Greeting

**Behavior:**
- Waves when player approaches within 5m
- Only waves once
- Plays greeting sound

```protobuf
StateTrigger {
  trigger_id: "trigger_npc_wave"
  entity_id: "npc_merchant_001"

  condition {
    if_near {
      target_entity_id: "player"
      distance: 5.0
      mode: HORIZONTAL_ONLY
      require_in_front: true
      field_of_view: 120.0
    }
  }

  on_enter {
    play_animation {
      animation_name: "wave_greeting"
      loop: false
      blend_time: 0.3
    }
    client_side: true
  }

  on_enter {
    delay: 0.5
    play_sound {
      sound_id: "npc_greeting_01"
      volume: 0.8
      spatial_mode: POSITIONAL_3D
      position { x: 10.0 y: 1.5 z: 5.0 }
    }
    client_side: true
  }

  on_exit {
    play_animation {
      animation_name: "idle"
      loop: true
      blend_time: 0.5
    }
    client_side: true
  }

  options {
    enabled: true
    one_shot: true
  }
}
```

**Result:**
```
Player @ 6m: Nothing
Player @ 5m: NPC waves instantly! (0ms)
            500ms later: "Hello!" sound
Player @ 6m: NPC returns to idle
```

---

### Example 2: Interactive Door

**Behavior:**
- Highlights on hover (when near)
- Opens on click (if unlocked)
- Auto-closes after 5 seconds

```protobuf
# Trigger 1: Hover highlight
StateTrigger {
  trigger_id: "trigger_door_hover"
  entity_id: "door_001"

  condition {
    and_condition {
      conditions {
        on_hover {
          require_raycast_hit: true
          hover_delay: 0.05
        }
      }
      conditions {
        if_near {
          target_entity_id: "player"
          distance: 3.0
        }
      }
    }
  }

  on_enter {
    set_color {
      color { r: 0.2 g: 0.6 b: 1.0 a: 0.3 }
      transition_time: 0.15
      property: "emissive"
    }
    client_side: true
  }

  on_exit {
    set_color {
      color { r: 0.0 g: 0.0 b: 0.0 a: 0.0 }
      transition_time: 0.15
      property: "emissive"
    }
    client_side: true
  }

  options {
    enabled: true
    retrigger_on_exit: true
  }
}

# Trigger 2: Click to open
StateTrigger {
  trigger_id: "trigger_door_click"
  entity_id: "door_001"

  condition {
    and_condition {
      conditions {
        on_click { button: 0 require_hover: true }
      }
      conditions {
        if_near { target_entity_id: "player" distance: 3.0 }
      }
      conditions {
        has_property {
          property_name: "is_locked"
          operator: EQUAL
          bool_value: false
        }
      }
    }
  }

  on_enter {
    play_animation {
      animation_name: "door_open"
      loop: false
    }
    client_side: true
  }

  on_enter {
    set_property {
      property_name: "is_open"
      bool_value: true
    }
    client_side: true
  }

  on_enter {
    play_sound {
      sound_id: "door_creak"
      volume: 0.6
      spatial_mode: POSITIONAL_3D
      position { x: 5.0 y: 1.0 z: 0.0 }
    }
    client_side: true
  }

  options {
    enabled: true
    cooldown: 0.5
  }
}

# Trigger 3: Auto-close
StateTrigger {
  trigger_id: "trigger_door_autoclose"
  entity_id: "door_001"

  condition {
    and_condition {
      conditions {
        has_property {
          property_name: "is_open"
          operator: EQUAL
          bool_value: true
        }
      }
      conditions {
        after_time {
          delay_seconds: 5.0
        }
      }
    }
  }

  on_enter {
    play_animation {
      animation_name: "door_close"
    }
    client_side: true
  }

  on_enter {
    set_property {
      property_name: "is_open"
      bool_value: false
    }
    client_side: true
  }

  options {
    enabled: true
    retrigger_on_exit: true
  }
}
```

**Result:**
```
Hover (near door): Highlight instantly (0ms)
Click: Door opens with animation (0ms visual feedback)
Wait 5s: Door closes automatically
```

---

### Example 3: Pressure Plate

**Behavior:**
- Depresses when stepped on
- Triggers linked door
- Releases when stepped off

```protobuf
StateTrigger {
  trigger_id: "trigger_plate_stand"
  entity_id: "pressure_plate_001"

  condition {
    on_stand {
      surface_entity_id: "pressure_plate_001"
      surface_tolerance: 0.2
      require_grounded: true
    }
  }

  # Depress animation
  on_enter {
    move_to {
      target_position { x: 0.0 y: -0.05 z: 0.0 }
      relative: true
    }
    duration: 0.15
    easing: "ease_out_cubic"
    client_side: true
  }

  on_enter {
    set_property {
      property_name: "is_pressed"
      bool_value: true
    }
    client_side: true
  }

  on_enter {
    play_sound {
      sound_id: "button_press"
      volume: 0.5
      spatial_mode: POSITIONAL_3D
      position { x: 8.0 y: 0.0 z: 2.0 }
    }
    client_side: true
  }

  # Release animation
  on_exit {
    move_to {
      target_position { x: 0.0 y: 0.05 z: 0.0 }
      relative: true
    }
    duration: 0.2
    easing: "ease_in_out_cubic"
    client_side: true
  }

  on_exit {
    set_property {
      property_name: "is_pressed"
      bool_value: false
    }
    client_side: true
  }

  options {
    enabled: true
    retrigger_on_exit: true
  }
}
```

**Result:**
```
Step on plate: Instantly depresses (0ms)
               Door opens
Step off: Instantly rises (0ms)
          Door closes
```

---

## Best Practices

### ✅ DO

**Use `client_side: true` for visual feedback**
```protobuf
on_enter {
  play_animation { }
  client_side: true  # Instant feedback
}
```

**Combine conditions for complex logic**
```protobuf
condition {
  and_condition {
    conditions { on_hover { } }
    conditions { if_near { } }
    conditions { has_property { } }
  }
}
```

**Use cooldowns to prevent spam**
```protobuf
options {
  cooldown: 0.5  # 500ms between triggers
}
```

**Chain triggers for complex behaviors**
```protobuf
# Trigger 1: Opens chest
on_enter {
  set_property {
    property_name: "is_open"
    bool_value: true
  }
}

# Trigger 2: Reacts to chest opening
condition {
  on_state_change {
    property_name: "is_open"
    bool_value: true
  }
}
```

### ❌ DON'T

**Don't use `client_side: false` for animations**
```protobuf
on_enter {
  play_animation { }
  client_side: false  # ❌ BAD: Network delay!
}
```

**Don't forget hover delay for stability**
```protobuf
condition {
  on_hover {
    hover_delay: 0.0  # ❌ BAD: Too sensitive!
  }
}
```

**Don't create trigger loops**
```protobuf
# ❌ BAD: Trigger 1 sets property_a
# Trigger 2 reacts to property_a, sets property_b
# Trigger 1 reacts to property_b... infinite loop!
```

---

## Performance Considerations

### Evaluation Cost

| Condition Type | Cost | Notes |
|----------------|------|-------|
| `ifNear` | Medium | Distance calculation |
| `onHover` | Low | Raycast already done |
| `onStand` | Medium | Collision check |
| `onStateChange` | Very Low | Event-based |
| `hasProperty` | Very Low | Simple lookup |
| `inFrustum` | Low | Cached frustum |

### Optimization Tips

**1. Use appropriate distance modes**
```protobuf
if_near {
  mode: HORIZONTAL_ONLY  # Faster than EUCLIDEAN
}
```

**2. Disable triggers when not needed**
```protobuf
options {
  enabled: false  # Disable when out of range
}
```

**3. Limit `while_active` actions**
```protobuf
# ✅ GOOD: One-time actions
on_enter { play_animation { } }

# ❌ BAD: Expensive per-frame action
while_active { run_script { } }
```

**4. Use composite conditions wisely**
```protobuf
# ✅ GOOD: Cheap condition first
and_condition {
  conditions { has_property { } }  # Very cheap
  conditions { if_near { } }       # More expensive
}
```

---

## Summary

### State Trigger System Benefits

- ⚡ **0ms response time**: Instant visual feedback
- 🎯 **Declarative**: Server defines rules, client executes
- 🔧 **Flexible**: Combine conditions and actions
- 📦 **Efficient**: Client-side evaluation
- 🎮 **Immersive**: Reactive, living world

### Common Use Cases

| Use Case | Conditions | Actions |
|----------|-----------|---------|
| **NPC Greeting** | `ifNear` | `playAnimation`, `playSound` |
| **Button Hover** | `onHover` | `setColor`, `showUI` |
| **Door Interaction** | `onClick` + `ifNear` | `playAnimation`, `setProperty` |
| **Pressure Plate** | `onStand` | `moveTo`, `playSound` |
| **Treasure Chest** | `onClick` + `ifNear` | `playAnimation`, `playEffect`, `spawnEntity` |
| **Proximity Sensor** | `inZone` | `playEffect`, `shakeCamera` |

---

## Next Steps

1. **Define triggers** for your interactive objects
2. **Test client-side execution** for instant feedback
3. **Combine conditions** for complex behaviors
4. **Monitor performance** using debug visualization
5. **Iterate on cooldowns** and delays for best feel

**Your world is now reactive and immersive!** 🎮

---

**See also:**
- [`cryo_state_triggers.proto`](proto/ecs/cryo_state_triggers.proto) - Protocol definition
- [`state_triggers_npc_example.pbtxt`](examples/state_triggers_npc_example.pbtxt) - NPC examples
- [`state_triggers_objects_example.pbtxt`](examples/state_triggers_objects_example.pbtxt) - Object examples
- [`RESILIENT_CLIENT_GUIDE.md`](RESILIENT_CLIENT_GUIDE.md) - Client state management