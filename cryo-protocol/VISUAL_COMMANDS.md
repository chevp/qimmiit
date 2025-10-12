# Cryo Visual Commands - Grafische Verben Übersicht

Alle grafisch relevanten Verben im Cryo-Protocol sind zentral in der Datei `proto/rendering_commands/cryo_visual_commands.proto` definiert.

## Haupt-Kategorien

### 🔄 Bewegung (Movement)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **move** | `MoveCommand` | Bewegt eine Entity im Raum (Position/Translation) |
| **move_to** | `MoveToCommand` | Bewegt Entity zu einer Zielposition mit Geschwindigkeit |
| **move_along_path** | `MoveAlongPathCommand` | Bewegt Entity entlang eines Pfades |

### 🔄 Rotation
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **rotate** | `RotateCommand` | Rotiert Entity oder UI-Element (Quaternion) |
| **rotate_euler** | `RotateEulerCommand` | Rotiert mit Euler-Winkeln |
| **rotate_around** | `RotateAroundCommand` | Rotiert um einen Pivot-Punkt |
| **spin** | `SpinCommand` | Kontinuierliche Rotation um eine Achse |

### 📏 Größenänderung (Scale)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **scale** | `ScaleCommand` | Skaliert Entity (pro Achse) |
| **scale_uniform** | `ScaleUniformCommand` | Uniforme Skalierung |
| **pulse_scale** | `PulseScaleCommand` | Pulsierende Größenanimation |

### 🎬 Animation
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **play_animation** | `PlayAnimationCommand` | Startet Animationssequenz (Laufanimation, Angriff, etc.) |
| **stop_animation** | `StopAnimationCommand` | Stoppt Animation |
| **pause_animation** | `PauseAnimationCommand` | Pausiert Animation |
| **resume_animation** | `ResumeAnimationCommand` | Setzt Animation fort |
| **set_animation_speed** | `SetAnimationSpeedCommand` | Ändert Abspielgeschwindigkeit |
| **blend_animations** | `BlendAnimationsCommand` | Blendet zwischen Animationen |

### ✨ Visuelle Effekte
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **trigger_effect** | `TriggerEffectCommand` | Löst visuellen Effekt aus (Partikel, Lichtblitz, Explosion) |
| **stop_effect** | `StopEffectCommand` | Beendet visuellen Effekt |
| **update_effect** | `UpdateEffectCommand` | Aktualisiert Effekt-Parameter |

### 🌟 Partikel
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **spawn_particle** | `SpawnParticleCommand` | Erzeugt Partikelsystem |
| **despawn_particle** | `DespawnParticleCommand` | Entfernt Partikelsystem |
| **update_particle** | `UpdateParticleCommand` | Aktualisiert Partikel-Parameter |
| **particle_burst** | `ParticleBurstCommand` | Sofortiger Partikel-Burst |

### 🎯 Icons
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **show_icon** | `ShowIconCommand` | Blendet Icon ein (Buff-Status, Indikator, etc.) |
| **hide_icon** | `HideIconCommand` | Blendet Icon aus |
| **update_icon** | `UpdateIconCommand` | Aktualisiert Icon (Position, Textur, Skalierung) |
| **pulse_icon** | `PulseIconCommand` | Pulsierendes Icon |

### 📝 Text
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **update_text** | `UpdateTextCommand` | Ändert Textinhalt (Health-Wert aktualisieren) |
| **update_text_color** | `UpdateTextColorCommand` | Ändert Textfarbe |
| **update_text_size** | `UpdateTextSizeCommand` | Ändert Textgröße |
| **typewriter_text** | `TypewriterTextCommand` | Typewriter-Effekt |

### 🌓 Fade (Ein-/Ausblenden)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **fade_in** | `FadeInCommand` | Einblenden mit Übergang |
| **fade_out** | `FadeOutCommand` | Ausblenden mit Übergang |
| **fade_to** | `FadeToCommand` | Zu bestimmter Deckkraft überblenden |
| **crossfade** | `CrossfadeCommand` | Überblenden zwischen zwei Targets |

### 💡 Hervorhebung (Highlight)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **highlight** | `HighlightCommand` | Hebt Element hervor (Glühen, Umriss, Silhouette) |
| **unhighlight** | `UnhighlightCommand` | Entfernt Hervorhebung |
| **update_highlight** | `UpdateHighlightCommand` | Aktualisiert Hervorhebungs-Parameter |

### ⚡ Flash (Blinken)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **flash** | `FlashCommand` | Blinken oder pulsierende Animation |
| **damage_flash** | `DamageFlashCommand` | Schadens-Flash-Effekt |
| **heal_flash** | `HealFlashCommand` | Heilungs-Flash-Effekt |

### 👁️ Sichtbarkeit
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **set_visibility** | `SetVisibilityCommand` | Sichtbarkeit ein-/ausschalten |
| **show** | `ShowCommand` | Element anzeigen |
| **hide** | `HideCommand` | Element verstecken |
| **toggle_visibility** | `ToggleVisibilityCommand` | Sichtbarkeit umschalten |

### 📊 Layering
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **move_to_front** | `MoveToFrontCommand` | UI-Element in den Vordergrund |
| **move_to_back** | `MoveToBackCommand` | UI-Element in den Hintergrund |
| **set_layer** | `SetLayerCommand` | Setzt Rendering-Layer |
| **set_render_order** | `SetRenderOrderCommand` | Setzt Render-Reihenfolge |

### 📳 Shake (Wackeln)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **shake** | `ShakeCommand` | Wackeln/Schütteln (z.B. bei Schaden) |
| **screen_shake** | `ScreenShakeCommand` | Bildschirm-Wackeln |
| **damage_shake** | `DamageShakeCommand` | Schadens-Wackel-Effekt |

### 📈 Fortschritt
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **update_progress** | `UpdateProgressCommand` | Aktualisiert Fortschrittsanzeige |
| **update_progress_bar** | `UpdateProgressBarCommand` | Aktualisiert Progress-Bar (Health, Cooldown) |
| **update_cooldown** | `UpdateCooldownCommand` | Aktualisiert Cooldown-Anzeige |
| **pulse_progress** | `PulseProgressCommand` | Pulsierende Fortschrittsanzeige |

### 🎨 Farbe
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **set_color** | `SetColorCommand` | Ändert Farbe (z.B. Rot bei Warnung) |
| **tint** | `TintCommand` | Färbt Element ein |
| **color_cycle** | `ColorCycleCommand` | Farbzyklus-Animation |
| **color_pulse** | `ColorPulseCommand` | Pulsierende Farbanimation |

### 🎭 Material
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **set_material_property** | `SetMaterialPropertyCommand` | Setzt Material-Property |
| **swap_material** | `SwapMaterialCommand` | Tauscht Material aus |
| **dissolve** | `DissolveCommand` | Auflösungs-Effekt |

### 🔊 Audio (gekoppelt mit Grafik)
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **play_sound** | `PlaySoundCommand` | Spielt Sound ab (oft gekoppelt mit Grafik) |
| **stop_sound** | `StopSoundCommand` | Stoppt Sound |
| **play_sound_effect** | `PlaySoundEffectCommand` | Spielt Sound-Effekt ab |

### 🎯 Composite Commands
| Befehl | Message | Beschreibung |
|--------|---------|--------------|
| **visual_sequence** | `VisualSequence` | Sequenz von visuellen Befehlen |
| **visual_group** | `VisualGroup` | Parallele Ausführung mehrerer Befehle |
| **batch_visual_commands** | `BatchVisualCommands` | Batch-Verarbeitung |

## Verwendung

Alle Commands verwenden ein einheitliches `VisualTarget` System:

```protobuf
message VisualTarget {
  oneof target {
    string entity_id = 1;           // Scene entity
    string ui_element_id = 2;       // UI element
    string gltf_node_path = 3;      // GLTF node path
  }
}
```

## Beispiele

### Bewegung mit Fade-In
```protobuf
// Entity bewegen
MoveCommand {
  target: { entity_id: "player" }
  position: { x: 10, y: 0, z: 5 }
  duration: 2.0
  easing: "ease_in_out_quad"
}

// Dabei einblenden
FadeInCommand {
  target: { entity_id: "player" }
  duration: 2.0
  target_opacity: 1.0
}
```

### Schadens-Feedback
```protobuf
// Flash-Effekt
DamageFlashCommand {
  target: { entity_id: "enemy" }
  intensity: 0.8
  duration: 0.2
}

// Shake
DamageShakeCommand {
  target: { entity_id: "enemy" }
  damage_amount: 50
  duration: 0.3
}

// Sound
PlaySoundEffectCommand {
  sfx_type: "hit"
  position: { x: 5, y: 1, z: 3 }
  volume: 0.7
}
```

### UI-Update
```protobuf
// Health-Bar aktualisieren
UpdateProgressBarCommand {
  progress_bar_id: "player_health"
  current_value: 75
  max_value: 100
  transition_duration: 0.3
  bar_color: { r: 1.0, g: 0.0, b: 0.0, a: 1.0 }
}

// Health-Text aktualisieren
UpdateTextCommand {
  text_element_id: "health_text"
  new_text: "75/100"
  transition_duration: 0.3
  transition: NUMBER_COUNT
}
```

### Buff-Icon anzeigen
```protobuf
ShowIconCommand {
  icon_id: "strength_buff"
  icon_type: "buff"
  icon_texture_uri: "asset://ui/icons/strength.png"
  parent_entity_id: "player"
  duration: 10.0
  fade_in_duration: 0.5
  scale: 1.0
}

// Pulsieren lassen
PulseIconCommand {
  icon_id: "strength_buff"
  min_scale: 0.9
  max_scale: 1.1
  frequency: 2.0
  duration: 10.0
}
```

## Datei-Struktur

```
cryo-protocol/
└── proto/
    ├── rendering_commands/
    │   └── cryo_visual_commands.proto   ← Alle visuellen Befehle hier
    ├── entity_scene_graph/
    │   ├── cryo_entity.proto
    │   ├── cryo_entity_state.proto      ← Verwendet visual commands
    │   └── ...
    ├── ui_system/
    │   ├── ui_animation.proto           ← Erweiterte UI-Animationen
    │   └── ...
    └── animation_simulation/
        └── cryo_animation.proto         ← 3D-Animationen
```

## Checkliste aller Verben

✅ **Basis-Verben** (Alle implementiert)
- ✅ move (MoveCommand, MoveToCommand, MoveAlongPathCommand)
- ✅ rotate (RotateCommand, RotateEulerCommand, RotateAroundCommand, SpinCommand)
- ✅ scale (ScaleCommand, ScaleUniformCommand, PulseScaleCommand)
- ✅ play_animation (PlayAnimationCommand)
- ✅ stop_animation (StopAnimationCommand)
- ✅ trigger_effect (TriggerEffectCommand)
- ✅ stop_effect (StopEffectCommand)
- ✅ show_icon (ShowIconCommand)
- ✅ hide_icon (HideIconCommand)
- ✅ update_text (UpdateTextCommand)
- ✅ fade_in (FadeInCommand)
- ✅ fade_out (FadeOutCommand)
- ✅ highlight (HighlightCommand)
- ✅ unhighlight (UnhighlightCommand)
- ✅ flash (FlashCommand)
- ✅ set_visibility (SetVisibilityCommand)
- ✅ move_to_front (MoveToFrontCommand)
- ✅ shake (ShakeCommand)
- ✅ update_progress (UpdateProgressCommand, UpdateProgressBarCommand, UpdateCooldownCommand)
- ✅ set_color (SetColorCommand)

✅ **Erweiterte Verben** (Alle implementiert)
- ✅ spawn_particle (SpawnParticleCommand)
- ✅ despawn_particle (DespawnParticleCommand)
- ✅ play_sound (PlaySoundCommand)
- ✅ stop_sound (StopSoundCommand)

## Integration

Die visuellen Befehle können verwendet werden in:

1. **Entity State Updates** (`cryo_entity_state.proto`)
2. **UI Animationen** (`ui_animation.proto`)
3. **Scripting Commands** (`cryo_scripting.proto`)
4. **Agent Commands** (`cryo_agends_commands.proto`)
5. **Scene Updates** (`cryo_scene_update.proto`)

Alle Befehle unterstützen:
- ⏱️ Duration-basierte Animationen
- 📊 Easing-Funktionen
- 🔄 Relative/absolute Transformationen
- 🎯 Flexible Target-Spezifikation (Entity, UI, GLTF-Node)