# Cryo Frame System - Quick Reference

## Frame-Typen auf einen Blick

| Frame Type | Beschreibung | Wann verwenden | Bandbreite | Latenz |
|------------|--------------|----------------|------------|---------|
| **FULL** (I-Frame) | Vollständiger Zustand | Alle N Frames (z.B. 60) | Hoch | Mittel |
| **DELTA** (P-Frame) | Nur Änderungen | Meiste Frames | Niedrig | Niedrig |
| **BIDIRECTIONAL** (B-Frame) | Interpoliert | Playback/Recording | Sehr niedrig | Hoch |
| **SKIP** | Kein Update | Frame-Dropping | Keine | Keine |

## Schnellstart

### 1. Stream erstellen (60 FPS)

```cpp
StreamConfig config;
config.set_target_fps(60.0);
config.set_keyframe_interval(60);  // I-Frame jede Sekunde
config.set_allow_frame_drop(true);
```

### 2. I-Frame senden (vollständiger Zustand)

```cpp
Frame full_frame;
full_frame.set_frame_number(0);
full_frame.set_type(Frame::FULL);

auto* group = full_frame.add_groups();
group->set_purpose("complete_scene_state");
group->mutable_data()->Add()->PackFrom(scene_snapshot);

stream->Write(full_frame);
```

### 3. P-Frame senden (nur Deltas)

```cpp
Frame delta_frame;
delta_frame.set_frame_number(1);
delta_frame.set_type(Frame::DELTA);
delta_frame.add_reference_frames(0);  // Basiert auf Frame 0

auto* group = delta_frame.add_groups();
group->set_purpose("entity_transforms");

for (auto& [entity_id, delta] : changed_entities) {
  group->mutable_data()->Add()->PackFrom(delta);
}

stream->Write(delta_frame);
```

### 4. Client empfängt Frames

```cpp
Frame frame;
while (stream->Read(&frame)) {
  if (frame.type() == Frame::FULL) {
    ApplyFullState(frame);
  } else if (frame.type() == Frame::DELTA) {
    ApplyDelta(frame);
  }
  RenderScene();
}
```

## Message-Groups: Häufige Purposes

| Purpose | Inhalt | Priorität |
|---------|--------|-----------|
| `"complete_scene_state"` | Vollständiger Szenen-Zustand | CRITICAL |
| `"entity_transforms"` | Entity-Transformationen | HIGH |
| `"camera_movement"` | Kamera-Updates | HIGH |
| `"material_properties"` | Material-Updates | NORMAL |
| `"shader_update"` | Shader-Änderungen | NORMAL |
| `"lighting_changes"` | Licht-Updates | NORMAL |
| `"particle_effects"` | Partikel-Updates | LOW |
| `"environment_update"` | Skybox/Fog-Updates | LOW |

## Empfohlene Einstellungen

### Live Streaming
```cpp
config.set_target_fps(60.0);
config.set_keyframe_interval(60);    // 1 Sekunde
config.set_allow_frame_drop(true);
compression.set_method(ZSTD);
compression.set_compression_level(5);
```

### Recording
```cpp
config.set_target_fps(60.0);
config.set_keyframe_interval(300);   // 5 Sekunden (bessere Kompression)
config.set_allow_frame_drop(false);
compression.set_method(ZSTD);
compression.set_compression_level(9);
```

### Low Bandwidth
```cpp
config.set_target_fps(30.0);
config.set_keyframe_interval(30);    // 1 Sekunde (häufigere Recovery)
config.set_allow_frame_drop(true);
compression.set_method(ZSTD);
compression.set_compression_level(9);
```

### Deterministic Replay
```cpp
config.set_target_fps(60.0);
config.set_keyframe_interval(60);
config.set_allow_frame_drop(false);  // Keine Frames droppen!
compression.set_method(NONE);        // Keine Kompression für Determinismus
```

## Bandbreiten-Rechner

```
I-Frame: ~500 KB
P-Frame: ~10 KB
FPS: 60

Pro Sekunde @ 60 FPS:
= 1 I-Frame + 59 P-Frames
= 500 KB + (59 × 10 KB)
= 1090 KB = 1.1 MB/s

Mit ZSTD (5:1 Kompression):
= 1.1 MB ÷ 5 = 220 KB/s

Ohne Frame-System:
= 60 × 500 KB = 30 MB/s

Einsparung: 99.3%!
```

## Playback-Befehle

### Play
```cpp
PlaybackControl play;
play.mutable_play()->set_from_frame(0);
play.mutable_play()->set_loop(false);
frame_service->ControlPlayback(play);
```

### Pause
```cpp
PlaybackControl pause;
pause.mutable_pause();
frame_service->ControlPlayback(pause);
```

### Seek
```cpp
PlaybackControl seek;
seek.mutable_seek()->set_frame_number(120);  // Frame 120
frame_service->ControlPlayback(seek);
```

### Speed
```cpp
PlaybackControl speed;
speed.mutable_set_speed()->set_speed(2.0);  // 2x speed
frame_service->ControlPlayback(speed);
```

### Step (Frame-by-frame)
```cpp
PlaybackControl step;
step.mutable_step()->set_frames(1);  // Nächster Frame
frame_service->ControlPlayback(step);
```

## Synchronisation

### Client Sync
```cpp
FrameSyncRequest sync;
sync.set_client_id("client_123");
sync.set_current_frame(42);

auto response = frame_service->SyncToFrame(sync);

if (response.status() == FrameSyncResponse::BEHIND) {
  // Catch up
  for (const auto& frame : response.catch_up().frames()) {
    ApplyFrame(frame);
  }
} else if (response.status() == FrameSyncResponse::DESYNC) {
  // Full resync
  ApplyFullState(response.full_resync().keyframe());
}
```

## Recording

### Start
```cpp
StartRecordingRequest request;
request.set_session_id("session_001");
request.mutable_config()->set_target_fps(60.0);
request.mutable_metadata()->set_title("My Session");

frame_service->StartRecording(request);
```

### Stop
```cpp
StopRecordingRequest stop;
stop.set_session_id("session_001");
stop.set_save_to_file(true);
stop.set_output_path("recordings/session_001.cryo");

frame_service->StopRecording(stop);
```

### Load & Play
```cpp
LoadRecordingRequest load;
load.set_file_path("recordings/session_001.cryo");

auto session = frame_service->LoadRecording(load);

PlaybackControl play;
play.mutable_play()->set_from_frame(0);
frame_service->ControlPlayback(play);
```

## Error Handling

### Packet Loss
```cpp
if (frame.type() == Frame::DELTA && !HasReferenceFrame(frame.reference_frames(0))) {
  LOG(WARNING) << "Missing reference, waiting for keyframe...";
  return;
}

if (frame.type() == Frame::FULL) {
  ApplyFullState(frame);
  in_sync_ = true;
}
```

### State Drift
```cpp
if (frame.metadata().is_keyframe()) {
  bytes computed = ComputeStateHash();
  bytes expected = frame.sync_point().state_hash();

  if (computed != expected) {
    RequestFullResync();
  }
}
```

## Performance-Tipps

### Buffer-Größe
```cpp
// Minimum: 2 * keyframe_interval
buffer.set_capacity(120);  // 2 Sekunden @ 60 FPS

// Empfohlen: 3 * keyframe_interval
buffer.set_capacity(180);  // 3 Sekunden @ 60 FPS
```

### Frame-Dropping
```cpp
if (buffer.size() > buffer.capacity() * 0.9) {
  // Alte P-Frames droppen
  DropOldDeltaFrames(buffer);

  // ABER: I-Frames niemals droppen!
  KeepKeyframes(buffer);
}
```

### Prioritäten
```cpp
// Kritisch (Player-Position)
group.set_priority(MessageGroup::CRITICAL);
group.set_atomic(true);

// Normal (Environment)
group.set_priority(MessageGroup::NORMAL);

// Low (Partikel)
group.set_priority(MessageGroup::LOW);
```

## Compression-Methoden

| Method | Speed | Ratio | Wann verwenden |
|--------|-------|-------|----------------|
| NONE | Instant | 1.0x | Deterministic replay |
| LZ4 | Very Fast | 2-3x | Low latency |
| ZLIB | Fast | 3-4x | Balance |
| **ZSTD** | Fast | 4-6x | **Empfohlen** |
| BROTLI | Slow | 6-8x | Archivierung |

## gRPC Service-Übersicht

```protobuf
service FrameService {
  // Streaming
  rpc SubscribeToFrames (SubscribeRequest) returns (stream Frame);
  rpc PublishFrames (stream Frame) returns (PublishResponse);
  rpc StreamFrames (stream Frame) returns (stream Frame);

  // Encoding/Decoding
  rpc EncodeFrame (EncodeFrameRequest) returns (EncodeFrameResponse);
  rpc DecodeFrame (DecodeFrameRequest) returns (DecodeFrameResponse);

  // Synchronization
  rpc SyncToFrame (FrameSyncRequest) returns (FrameSyncResponse);
  rpc GetSyncPoints (GetSyncPointsRequest) returns (GetSyncPointsResponse);

  // Playback
  rpc ControlPlayback (PlaybackControl) returns (PlaybackStatus);
  rpc GetPlaybackStatus (GetPlaybackStatusRequest) returns (PlaybackStatus);

  // Recording
  rpc StartRecording (StartRecordingRequest) returns (RecordingStatus);
  rpc StopRecording (StopRecordingRequest) returns (RecordingStatus);
  rpc LoadRecording (LoadRecordingRequest) returns (RecordingSession);
}
```

## Cheat Sheet

```cpp
// I-Frame erstellen
Frame full;
full.set_type(Frame::FULL);
// → Vollständiger Zustand

// P-Frame erstellen
Frame delta;
delta.set_type(Frame::DELTA);
delta.add_reference_frames(previous_frame_number);
// → Nur Änderungen

// Frame komprimieren
compression.set_method(ZSTD);
compression.set_compression_level(5);
// → 5:1 Kompression

// Keyframe-Interval setzen
config.set_keyframe_interval(60);
// → I-Frame alle 60 Frames

// Frame-Dropping erlauben
config.set_allow_frame_drop(true);
// → Bei Überlastung Frames droppen

// Sync bei Packet-Loss
if (missing_frames) {
  SyncToFrame(current_frame);
  // → Full resync
}
```

## Use Cases

| Use Case | target_fps | keyframe_interval | allow_frame_drop | compression |
|----------|------------|-------------------|------------------|-------------|
| Live Streaming | 60 | 60 | ✅ | ZSTD (5) |
| Recording | 60 | 300 | ❌ | ZSTD (9) |
| Low Bandwidth | 30 | 30 | ✅ | ZSTD (9) |
| Deterministic | 60 | 60 | ❌ | NONE |
| Mobile | 30 | 30 | ✅ | LZ4 |

---

**Siehe auch**:
- [FRAME_SYSTEM.md](FRAME_SYSTEM.md) - Vollständige Dokumentation
- [cryo_frame.proto](cryo_frame.proto) - Protobuf Definition
