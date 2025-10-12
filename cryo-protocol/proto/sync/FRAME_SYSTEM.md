# Cryo Frame System

Frame-basiertes Synchronisationsprotokoll für zeitliche/sequenzielle Optimierung, inspiriert von Video-Codec-Konzepten.

## Konzept

Das Frame-System verwendet Video-Codec-Konzepte (I-Frames, P-Frames, B-Frames) für effiziente Scene-Updates:

```
Frame 0:  FULL  (I-Frame)  ← Recovery Point (vollständiger Zustand)
Frame 1:  DELTA (P-Frame)  ← Nur Änderungen seit Frame 0
Frame 2:  DELTA (P-Frame)  ← Nur Änderungen seit Frame 1
Frame 3:  DELTA (P-Frame)  ← Nur Änderungen seit Frame 2
...
Frame 60: FULL  (I-Frame)  ← Recovery Point (alle 1 Sekunde @ 60 FPS)
```

## Frame-Typen

### 1. FULL Frame (I-Frame)
**Vollständiger Zustand** - Kann unabhängig dekodiert werden

```protobuf
Frame {
  frame_number: 0
  type: FULL
  groups: [
    {
      purpose: "complete_scene_state"
      data: [SceneSnapshot, MaterialState, EntityState, ...]
    }
  ]
}
```

**Verwendung**:
- Recovery Point bei Packet-Loss
- Neuer Client kann sofort einsteigen
- Keyframe für Seeking
- Alle N Frames (z.B. alle 60 Frames @ 60 FPS = jede Sekunde)

**Vorteile**:
- ✅ Unabhängig dekodierbar
- ✅ Keine Abhängigkeiten
- ✅ Fehlertoleranz

**Nachteile**:
- ❌ Größer (mehr Bandbreite)
- ❌ Niedrigere Frequenz

### 2. DELTA Frame (P-Frame)
**Nur Änderungen** - Referenziert vorherigen Frame

```protobuf
Frame {
  frame_number: 1
  type: DELTA
  reference_frames: [0]  // Basiert auf Frame 0
  groups: [
    {
      purpose: "entity_transform_updates"
      data: [EntityTransformDelta, MaterialPropertyDelta, ...]
    }
  ]
}
```

**Verwendung**:
- Hohe Frequenz (60-120 FPS)
- Minimale Bandbreite
- Kleine Änderungen

**Vorteile**:
- ✅ Klein (minimale Bandbreite)
- ✅ Hohe Frequenz möglich
- ✅ Effizient für kleine Änderungen

**Nachteile**:
- ❌ Benötigt Referenz-Frame
- ❌ Fehler propagieren
- ❌ Akkumulierte Drift möglich

### 3. BIDIRECTIONAL Frame (B-Frame)
**Interpoliert zwischen zwei Frames**

```protobuf
Frame {
  frame_number: 5
  type: BIDIRECTIONAL
  reference_frames: [4, 8]  // Referenziert Frame 4 und 8
  groups: [...]
}
```

**Verwendung**:
- Noch höhere Kompression
- Playback/Recording
- Temporale Interpolation

**Vorteile**:
- ✅ Höchste Kompression
- ✅ Smooth playback

**Nachteile**:
- ❌ Höhere Latenz (braucht zukünftigen Frame)
- ❌ Nur für Playback geeignet, nicht Live

### 4. SKIP Frame
**Kein Update** - Frame-Dropping bei Überlastung

```protobuf
Frame {
  frame_number: 10
  type: SKIP
}
```

## Message Groups

Thematische Gruppierung von zusammengehörigen Updates:

```protobuf
MessageGroup {
  purpose: "scene_shader_update"
  priority: HIGH
  atomic: true        // Alle Messages oder keine
  ordered: true       // Reihenfolge wichtig
  data: [
    SetUniform { uniform_name: "u_time", value: 1.5 },
    SetUniform { uniform_name: "u_color", value: (1.0, 0.0, 0.0) },
    UpdateShaderLayer { layer_id: "fog", opacity: 0.8 }
  ]
}
```

**Häufige Purposes**:
- `"scene_shader_update"` - Shader-Änderungen
- `"camera_movement"` - Kamera-Updates
- `"entity_transforms"` - Entity-Transformationen
- `"material_properties"` - Material-Updates
- `"lighting_changes"` - Licht-Updates
- `"environment_update"` - Environment-Updates

## Verwendungsbeispiele

### 1. Real-time Scene Streaming (60 FPS)

**Server**:
```cpp
// Initialer I-Frame (vollständiger Zustand)
Frame full_frame;
full_frame.set_frame_number(0);
full_frame.set_type(Frame::FULL);
full_frame.set_timestamp_ms(0);

auto* group = full_frame.add_groups();
group->set_purpose("complete_scene_state");
group->mutable_data()->Add()->PackFrom(scene_snapshot);

stream->Write(full_frame);

// Folgende P-Frames (nur Deltas)
for (int i = 1; i < 60; i++) {
  Frame delta_frame;
  delta_frame.set_frame_number(i);
  delta_frame.set_type(Frame::DELTA);
  delta_frame.add_reference_frames(i - 1);
  delta_frame.set_timestamp_ms(i * 16.67);  // 60 FPS

  auto* group = delta_frame.add_groups();
  group->set_purpose("entity_transforms");

  // Nur geänderte Entities
  for (auto& [entity_id, delta] : changed_entities) {
    group->mutable_data()->Add()->PackFrom(delta);
  }

  stream->Write(delta_frame);
}

// Nach 60 Frames: Nächster I-Frame
Frame keyframe;
keyframe.set_frame_number(60);
keyframe.set_type(Frame::FULL);
// ...
stream->Write(keyframe);
```

**Client**:
```cpp
// Empfangen und Dekodieren
Frame frame;
while (stream->Read(&frame)) {
  if (frame.type() == Frame::FULL) {
    // Vollständiger Zustand → direkt anwenden
    ApplyFullState(frame);
    last_full_frame_ = frame;
  } else if (frame.type() == Frame::DELTA) {
    // Delta → auf letzten Zustand anwenden
    ApplyDelta(frame);
  }

  RenderScene();
}
```

### 2. Session Recording & Playback

**Recording**:
```cpp
// Start Recording
StartRecordingRequest request;
request.set_session_id("gameplay_session_001");
request.mutable_config()->set_target_fps(60.0);
request.mutable_config()->set_keyframe_interval(60);
request.mutable_metadata()->set_title("Gameplay Session");

auto status = frame_service->StartRecording(request);

// Frames werden automatisch aufgezeichnet
// ...

// Stop Recording
StopRecordingRequest stop;
stop.set_session_id("gameplay_session_001");
stop.set_save_to_file(true);
stop.set_output_path("recordings/session_001.cryo");

frame_service->StopRecording(stop);
```

**Playback**:
```cpp
// Load Recording
LoadRecordingRequest load;
load.set_file_path("recordings/session_001.cryo");

RecordingSession session = frame_service->LoadRecording(load);

// Play
PlaybackControl play;
play.mutable_play()->set_from_frame(0);
play.mutable_play()->set_loop(false);

frame_service->ControlPlayback(play);

// Seek to specific frame
PlaybackControl seek;
seek.mutable_seek()->set_frame_number(120);  // Frame 120 (2 Sekunden @ 60 FPS)

frame_service->ControlPlayback(seek);

// Slow motion (0.5x speed)
PlaybackControl speed;
speed.mutable_set_speed()->set_speed(0.5);

frame_service->ControlPlayback(speed);
```

### 3. Network Synchronization mit Recovery

**Client verliert Verbindung und reconnected**:

```cpp
// Client reconnected
FrameSyncRequest sync_request;
sync_request.set_client_id("client_123");
sync_request.set_current_frame(42);
sync_request.set_current_state_hash(computed_hash);

FrameSyncResponse response = frame_service->SyncToFrame(sync_request);

if (response.status() == FrameSyncResponse::BEHIND) {
  // Client ist hinterher → catch up
  auto catch_up = response.catch_up();

  for (const auto& frame : catch_up.frames()) {
    ApplyFrame(frame);
  }

  current_frame_ = catch_up.target_frame();

} else if (response.status() == FrameSyncResponse::DESYNC) {
  // State-Hashes stimmen nicht → full resync
  auto full_resync = response.full_resync();

  ApplyFullState(full_resync.keyframe());
  current_frame_ = full_resync.frame_number();
}
```

### 4. Deterministic Replay

```cpp
// Deterministic replay für Multiplayer-Debugging
SubscribeRequest sub;
sub.set_stream_id("multiplayer_session_001");
sub.set_from_frame(0);  // Von Anfang an

auto stream = frame_service->SubscribeToFrames(sub);

Frame frame;
while (stream->Read(&frame)) {
  // Frame-by-frame replay mit deterministischer Reihenfolge
  ProcessFrame(frame);

  // Verify state at sync points
  if (frame.metadata().is_keyframe()) {
    bytes computed_hash = ComputeStateHash();
    bytes expected_hash = GetExpectedHash(frame.frame_number());

    if (computed_hash != expected_hash) {
      LOG(ERROR) << "Desync at frame " << frame.frame_number();
      // Debugging: Zustand ist nicht deterministisch
    }
  }
}
```

## Konfiguration

### StreamConfig

```protobuf
StreamConfig {
  target_fps: 60.0              // Ziel-Frame-Rate
  keyframe_interval: 60         // I-Frame alle 60 Frames (= 1 Sekunde)
  allow_frame_drop: true        // Frame-Dropping bei Überlastung
  max_reference_distance: 5     // Max 5 Frames zurück referenzieren
}
```

**Empfohlene Einstellungen**:

| Use Case | target_fps | keyframe_interval | allow_frame_drop |
|----------|------------|-------------------|------------------|
| **Live Streaming** | 60 | 60 | true |
| **Recording** | 60 | 60 | false |
| **Slow Network** | 30 | 30 | true |
| **Deterministic Replay** | 60 | 60 | false |
| **Low Bandwidth** | 30 | 120 | true |

### Compression

```protobuf
FrameCompression {
  method: ZSTD
  compression_level: 5  // 1-9 (höher = bessere Kompression, langsamer)
}
```

**Compression Methods**:
- `NONE` - Keine Kompression (niedrige Latenz)
- `LZ4` - Schnell, moderate Kompression
- `ZLIB` - Balance zwischen Speed und Ratio
- `ZSTD` - Beste Kompression, moderate Speed (empfohlen)
- `BROTLI` - Höchste Kompression, langsam

## Performance

### Bandbreite-Schätzung

**Szenario: 60 FPS Streaming**

```
I-Frame (FULL):   ~500 KB  (alle 60 Frames)
P-Frame (DELTA):  ~10 KB   (59 Frames)

Pro Sekunde:
= 1 * 500 KB + 59 * 10 KB
= 500 KB + 590 KB
= 1090 KB = ~1.1 MB/s

Mit ZSTD Compression (5:1):
= 1.1 MB / 5 = ~220 KB/s
```

**Vergleich ohne Frame-System**:
```
Ohne Optimierung: 60 * 500 KB = 30 MB/s
Mit Frame-System:  220 KB/s

Einsparung: 99.3%!
```

### Latenz

| Frame Type | Latency |
|------------|---------|
| FULL (I-Frame) | ~5-10 ms (encoding) |
| DELTA (P-Frame) | ~1-2 ms (encoding) |
| BIDIRECTIONAL (B-Frame) | ~10-20 ms (encoding) |

### Buffer-Size Empfehlungen

```protobuf
FrameBuffer {
  capacity: 120  // 2 Sekunden @ 60 FPS
}
```

**Regeln**:
- Minimum: `2 * keyframe_interval` (2 I-Frames)
- Empfohlen: `3 * keyframe_interval` (3 I-Frames)
- Für niedrige Latenz: `keyframe_interval` (1 I-Frame)

## Error Handling

### Packet Loss

**Problem**: Frame verloren → nachfolgende P-Frames können nicht dekodiert werden

**Lösung 1**: Auf nächsten I-Frame warten
```cpp
if (frame.type() == Frame::DELTA && !HasReferenceFrame(frame.reference_frames(0))) {
  // Referenz fehlt → Frame überspringen
  LOG(WARNING) << "Missing reference frame, waiting for keyframe...";
  return;
}

if (frame.type() == Frame::FULL) {
  // I-Frame empfangen → Recovery
  ApplyFullState(frame);
  in_sync_ = true;
}
```

**Lösung 2**: Server um Resync bitten
```cpp
FrameSyncRequest sync;
sync.set_current_frame(last_received_frame_);

auto response = frame_service->SyncToFrame(sync);
ApplyFullState(response.full_resync().keyframe());
```

### State Drift

**Problem**: Akkumulierte Rundungsfehler bei P-Frames

**Lösung**: Periodische I-Frames mit State-Hash-Verifikation
```cpp
if (frame.metadata().is_keyframe()) {
  bytes computed_hash = ComputeStateHash();
  bytes expected_hash = frame.sync_point().state_hash();

  if (computed_hash != expected_hash) {
    // Drift detected → Full resync
    RequestFullResync();
  }
}
```

## Best Practices

### 1. Keyframe-Interval wählen

```cpp
// Regel: target_fps * N Sekunden
StreamConfig config;
config.set_target_fps(60.0);

// Für Live-Streaming: 1-2 Sekunden
config.set_keyframe_interval(60);   // 1 Sekunde

// Für Recording: 5-10 Sekunden (bessere Kompression)
config.set_keyframe_interval(300);  // 5 Sekunden

// Für unreliable Network: 0.5-1 Sekunde (häufigere Recovery-Points)
config.set_keyframe_interval(30);   // 0.5 Sekunden
```

### 2. Message-Groups sinnvoll verwenden

```cpp
// ✅ GOOD: Logisch gruppiert
MessageGroup camera_group;
camera_group.set_purpose("camera_movement");
camera_group.add_data()->PackFrom(camera_pos_delta);
camera_group.add_data()->PackFrom(camera_rot_delta);
camera_group.add_data()->PackFrom(camera_fov_delta);

// ❌ BAD: Zu granular (overhead)
MessageGroup pos_group;
pos_group.set_purpose("camera_position");
pos_group.add_data()->PackFrom(camera_pos_delta);

MessageGroup rot_group;
rot_group.set_purpose("camera_rotation");
rot_group.add_data()->PackFrom(camera_rot_delta);
```

### 3. Prioritäten setzen

```cpp
// Kritische Updates (z.B. Player-Position)
MessageGroup critical_group;
critical_group.set_priority(MessageGroup::CRITICAL);
critical_group.set_atomic(true);

// Normale Updates (z.B. Environment)
MessageGroup normal_group;
normal_group.set_priority(MessageGroup::NORMAL);

// Low-Priority (z.B. Partikel-Effekte)
MessageGroup low_group;
low_group.set_priority(MessageGroup::LOW);
```

### 4. Frame-Dropping intelligent handhaben

```cpp
if (buffer.size() > buffer.capacity() * 0.9) {
  // Buffer fast voll → ältere P-Frames droppen
  DropOldDeltaFrames(buffer);

  // Aber: I-Frames NIEMALS droppen!
  KeepKeyframes(buffer);
}
```

## Integration mit anderen Cryo-Systemen

### Mit DeltaUpdate

```cpp
// Frame enthält DeltaUpdate
Frame frame;
frame.set_type(Frame::DELTA);

MessageGroup group;
group.set_purpose("entity_transforms");
group.mutable_data()->Add()->PackFrom(delta_update);

*frame.add_groups() = group;
```

### Mit Keyframes (Animation)

```cpp
// Frame enthält Animation-Keyframes
Frame frame;

MessageGroup group;
group.set_purpose("animation_update");
group.mutable_data()->Add()->PackFrom(animation_keyframe);

*frame.add_groups() = group;
```

### Mit Shader-Updates

```cpp
// Frame enthält Shader-Update-Groups
Frame frame;

MessageGroup group;
group.set_purpose("shader_fog_update");
group.mutable_data()->Add()->PackFrom(apply_fog);

*frame.add_groups() = group;
```

## Zusammenfassung

### Vorteile des Frame-Systems

✅ **Bandbreiten-Effizienz**: 99%+ Einsparung durch P-Frames
✅ **Fehlertoleranz**: I-Frames als Recovery-Points
✅ **Skalierbar**: 60+ FPS möglich
✅ **Flexibel**: Verschiedene Frame-Typen für verschiedene Use-Cases
✅ **Recording**: Built-in Support für Session-Recording
✅ **Deterministic**: State-Hashes für Verifikation
✅ **Kompatibel**: Integriert mit allen Cryo-Systemen

### Use Cases

- ✅ Real-time Scene Streaming (60 FPS)
- ✅ Session Recording & Playback
- ✅ Network Synchronization
- ✅ Deterministic Replay
- ✅ Multiplayer State Sync
- ✅ Scene State History
- ✅ Low-Bandwidth Streaming

**Das Frame-System ist produktionsreif!** 🚀
