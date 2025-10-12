# Cryo-Protocol Structure

Diese Struktur folgt den 12 Spezialisierungen einer modernen Rendering Engine.

## 1. ECS (Entity-Component-System)
**Zweck**: Logische Struktur der Szene (was existiert, wie es sich bewegt)

**Dateien**:
- `cryo_entity.proto` - Entity-Definitionen
- `cryo_entity_state.proto` - Entity-Zustände
- `cryo_entity_hierarchy.proto` - Entity-Hierarchien
- `cryo_create_entity.proto` - Entity erstellen
- `cryo_delete_entity.proto` - Entity löschen
- `cryo_move_entity.proto` - Entity bewegen
- `cryo_load_entity.proto` - Entity laden
- `cryo_unload_entity.proto` - Entity entladen
- `cryo_scripting.proto` - Scripting-Komponenten

**Kommunikation**: `CreateEntity`, `AttachComponent`, `UpdateComponent`

---

## 2. Shader/Material (Shader-Graph)
**Zweck**: Beschreibt, wie Objekte aussehen

**Dateien**:
- `cryo_asset_shader.proto` - Shader-Definitionen
- `cryo_asset_material.proto` - Material-Definitionen
- `cryo_material_commands.proto` - Material-Befehle

**Kommunikation**: `SetUniform`, `UpdateShaderGraph`, `ApplyMaterial`

---

## 3. Scene Graph (Hierarchien)
**Zweck**: Strukturiert Objekte räumlich oder logisch

**Dateien**:
- `cryo_scene_graph.proto` - Scene-Graph-Definitionen
- `cryo_scene_graph_operations.proto` - Scene-Graph-Operationen

**Kommunikation**: `AttachNode`, `DetachNode`, `SetTransform`

---

## 4. Render Pipeline (Pass Management)
**Zweck**: Steuert die Abfolge von Rendering-Schritten

**Dateien**:
- `cryo_render_settings.proto` - Render-Einstellungen
- `cryo_visual_commands.proto` - Visuelle Befehle

**Kommunikation**: `DefinePass`, `SetPassOrder`, `BindRenderTarget`

---

## 5. Camera/Viewport
**Zweck**: Bestimmt den Blickwinkel und die Frustum-Geometrie

**Dateien**:
- `cryo_camera.proto` - Kamera-Definitionen

**Kommunikation**: `SetCamera`, `MoveCamera`, `UpdateProjection`

---

## 6. Lighting/Environment
**Zweck**: Beleuchtung und globale Effekte

**Dateien**:
- `cryo_lighting.proto` - Licht-Definitionen

**Kommunikation**: `AddLight`, `UpdateLight`, `SetSkybox`, `SetEnvironmentProbe`

---

## 7. Animation/Skinning
**Zweck**: Bewegung innerhalb von Modellen

**Dateien**:
- `cryo_animation.proto` - Animation-Definitionen
- `cryo_physics.proto` - Physik-Simulation
- `particle_system.proto` - Partikel-Systeme

**Kommunikation**: `PlayAnimation`, `SetBoneTransform`, `SetMorphTargetWeight`

---

## 8. Asset Management (Resources)
**Zweck**: Lädt & verwaltet Ressourcen

**Dateien**:
- `cryo_asset_gltf.proto` - GLTF-Assets
- `cryo_asset_gltf_binary.proto` - GLTF-Binary-Daten
- `cryo_asset_gltf_commands.proto` - GLTF-Befehle
- `cryo_asset_texture.proto` - Textur-Assets
- `cryo_asset_mesh.proto` - Mesh-Assets

**Kommunikation**: `LoadAsset`, `UnloadAsset`, `GetAssetState`

---

## 9. Rendering 2D/UI
**Zweck**: Zeichnet HUD, Overlays, Text, Sprites

**Dateien**:
- UI System (13 Dateien):
  - `ui_base.proto` - UI-Basis-Typen
  - `ui_text.proto` - Text-Komponenten
  - `ui_icon.proto` - Icon-Komponenten
  - `ui_button.proto` - Button-Komponenten
  - `ui_panel.proto` - Panel-Komponenten
  - `ui_statusbar.proto` - Statusbar-Komponenten
  - `ui_overlay.proto` - Overlay-Komponenten
  - `ui_3d_text.proto` - 3D-Text
  - `ui_3d_icon.proto` - 3D-Icons
  - `ui_animation.proto` - UI-Animationen
  - `ui_state.proto` - UI-Zustand
  - `ui_input.proto` - UI-Input
  - `ui_hierarchy.proto` - UI-Hierarchien
  - `ui_commands.proto` - UI-Befehle
  - `ui_system.proto` - UI-System
- 2D Rendering (4 Dateien):
  - `sprite_2d.proto` - Sprite-System
  - `tile_2d.proto` - Tilemap-System
  - `atlas_2d.proto` - Texture-Atlas
  - `sprite_commands.proto` - Sprite-Befehle

**Kommunikation**: `DrawSprite`, `UpdateText`, `ShowTooltip`, `SetUITransform`

---

## 10. Postprocessing (Effects)
**Zweck**: Bildnachbearbeitung

**Dateien**:
- `cryo_post_processing.proto` - Postprocessing-Effekte

**Kommunikation**: `EnableEffect`, `SetEffectParameter`, `ReorderPostProcess`

---

## 11. Render Targets (Output Control)
**Zweck**: Zielorte für Renderings

**Dateien**:
- *(noch keine - Platzhalter für zukünftige Entwicklung)*

**Kommunikation**: `CreateRenderTarget`, `SetOutputResolution`

---

## 12. Debugging/Profiling
**Zweck**: Tools & Analysezwecke

**Dateien**:
- `debug.proto` - Debug-Befehle
- `telemetry.proto` - Telemetrie-Daten

**Kommunikation**: `GetRenderStats`, `HighlightEntity`, `CaptureFrame`

---

## Zusätzliche Kategorien

### Sync (Scene Update & Synchronization)
**Zweck**: State-Synchronisation und differentielle Updates

**Dateien**:
- `cryo_scene_update.proto` - Scene-Updates
- `cryo_sync.proto` - Synchronisation
- `batch_commands.proto` - Batch-Befehle
- `scene_update_delta.proto` - Delta-Updates
- `scene_keyframes.proto` - Keyframe-Animationen
- `scene_sequences.proto` - Befehlssequenzen
- `scene_state_sync.proto` - State-Versionierung
- `cryo_frame.proto` - Frame-basierte Synchronisation (I-Frame/P-Frame) ⭐ NEU
- `cryo_network.proto` - Network-Streaming-Protokoll ⭐ NEU

**Kommunikation**: `ApplyDelta`, `TakeSnapshot`, `SyncState`, `PlaySequence`, `SubscribeToFrames`, `EncodeFrame`

---

### Agent Commands
**Zweck**: Hochlevel-Befehle für autonome Agenten

**Dateien**:
- `cryo_agends_commands.proto` - Allgemeine Agent-Befehle
- `cryo_move.proto` - Bewegungsbefehle
- `cryo_pickup.proto` - Aufheben-Befehle
- `cryo_drop.proto` - Ablegen-Befehle
- `cryo_wait.proto` - Warten-Befehle

**Kommunikation**: `MoveAgent`, `PickupObject`, `DropObject`, `Wait`

---

## Common
**Zweck**: Gemeinsame Typen für alle Kategorien

**Dateien**:
- `cryo_common.proto` - Gemeinsame Basis-Typen (Vec2, Vec3, Vec4, Transform, Color, etc.)

---

## Import-Konventionen

Alle Imports verwenden relative Pfade vom `proto/` Verzeichnis:

```protobuf
// Beispiel: Importieren von Common-Typen
import "cryo_common.proto";

// Beispiel: Importieren aus anderer Kategorie
import "ecs/cryo_entity.proto";
import "shader_material/cryo_asset_material.proto";
import "sync/scene_state_sync.proto";
```

---

## Vorteile dieser Struktur

1. **Klare Trennung der Verantwortlichkeiten** - Jede Kategorie hat einen eindeutigen Zweck
2. **Skalierbarkeit** - Neue Features können leicht der passenden Kategorie hinzugefügt werden
3. **Wartbarkeit** - Entwickler finden schnell die relevanten Proto-Dateien
4. **Modularität** - Kategorien können unabhängig voneinander entwickelt werden
5. **Standardisierung** - Folgt bewährten Engine-Architekturen (Unity, Unreal, etc.)