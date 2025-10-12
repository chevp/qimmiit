# Cryo Frame Protocol Examples

This directory contains example frame sequences demonstrating the **Cryo Frame Protocol** for real-time scene streaming and synchronization.

## Frame Sequence Overview

The example sequence shows a typical 60 FPS streaming scenario over 5 frames (~66ms of real-time):

### Frame 000: FULL Frame (I-Frame / Keyframe)
**File:** `frame_000.frame.pbtxt`
**Type:** `FULL` (I-Frame)
**Timestamp:** 0ms

**Complete scene state including:**
- Camera initial position and orientation
- All scene entities (3 objects)
- Material definitions
- No compression (for fast random access)

**Use cases:**
- Stream entry point for new clients
- Recovery point after packet loss
- Deterministic replay starting point
- State synchronization checkpoint

---

### Frame 001: DELTA Frame (P-Frame)
**File:** `frame_001.frame.pbtxt`
**Type:** `DELTA` (P-Frame)
**Timestamp:** 16ms (+16.67ms @ 60 FPS)
**References:** Frame 0

**Changes only:**
- Camera rotation delta: `+0.01 rad` on X-axis
- Entity `entity_003` position delta: `+0.1` on X-axis

**Compression:** ZSTD level 3 (1.5x ratio)

---

### Frame 002: DELTA Frame (P-Frame)
**File:** `frame_002.frame.pbtxt`
**Type:** `DELTA` (P-Frame)
**Timestamp:** 33ms (+16.67ms @ 60 FPS)
**References:** Frame 1

**Changes:**
- Camera continues rotating
- Entity continues moving
- **Material animation:** `mat_metal_001` roughness: `0.3 → 0.35`

**Compression:** ZSTD level 3 (1.44x ratio)

---

### Frame 003: DELTA Frame (P-Frame)
**File:** `frame_003.frame.pbtxt`
**Type:** `DELTA` (P-Frame)
**Timestamp:** 50ms (+16.67ms @ 60 FPS)
**References:** Frame 2

**Changes:**
- Camera continues rotating
- Entity continues moving
- Material roughness continues: `0.35 → 0.4`
- **Lighting change:** Directional light intensity: `1.0 → 0.92` (sun fade)

**Compression:** ZSTD level 3 (1.47x ratio)

---

### Frame 004: DELTA Frame (P-Frame)
**File:** `frame_004.frame.pbtxt`
**Type:** `DELTA` (P-Frame)
**Timestamp:** 66ms (+16.67ms @ 60 FPS)
**References:** Frame 3

**Changes:**
- Camera continues rotating
- Original entity continues moving
- Material roughness: `0.4 → 0.45`
- Light intensity: `0.92 → 0.85`
- **New entity spawned:** `entity_004` (sphere with emissive material)

**Compression:** ZSTD level 3 (1.42x ratio)

---

## Frame Type Distribution

| Frame Type | Count | Percentage | Purpose |
|------------|-------|------------|---------|
| FULL (I-Frame) | 1 | 20% | Keyframe for synchronization |
| DELTA (P-Frame) | 4 | 80% | Incremental updates |

**Keyframe interval:** Every 60 frames (1 second @ 60 FPS)

---

## Compression Statistics

| Metric | Value |
|--------|-------|
| Frame 0 (FULL) | No compression (4096 bytes) |
| Avg Delta Frame Size | ~353 bytes compressed |
| Avg Compression Ratio | 1.45x |
| Total Sequence Size | ~5.5 KB (uncompressed: ~6.6 KB) |

---

## Message Groups Used

The examples demonstrate various message group purposes:

1. **`camera_movement`** - Camera transform updates (HIGH priority)
2. **`entity_transform_update`** - Object position/rotation changes (NORMAL priority)
3. **`material_property_change`** - Material parameter animation (NORMAL priority)
4. **`light_intensity_change`** - Lighting updates (HIGH priority)
5. **`entity_creation`** - New object spawning (CRITICAL priority)
6. **`scene_object_creation`** - Initial scene setup (HIGH priority, atomic)
7. **`camera_initial_state`** - Camera initialization (CRITICAL priority, atomic)
8. **`material_initial_state`** - Material definitions (NORMAL priority)

---

## Typical Streaming Patterns

### Pattern 1: 60 FPS with Keyframes Every Second
```
Frame 0:  FULL  ← Keyframe
Frame 1:  DELTA
Frame 2:  DELTA
...
Frame 59: DELTA
Frame 60: FULL  ← Next keyframe (recovery point)
Frame 61: DELTA
...
```

### Pattern 2: Low-Bandwidth Streaming (Keyframes Every 5 Seconds)
```
Frame 0:   FULL  ← Keyframe
Frame 1:   DELTA
...
Frame 299: DELTA
Frame 300: FULL  ← Next keyframe (5 seconds @ 60 FPS)
```

### Pattern 3: High-Reliability Recording (Frequent Keyframes)
```
Frame 0:  FULL  ← Every 30 frames
Frame 1:  DELTA
...
Frame 29: DELTA
Frame 30: FULL  ← Every 0.5 seconds @ 60 FPS
```

---

## Use Cases

### 1. **Real-Time Scene Streaming**
- Server sends frames to clients at 60 FPS
- Clients decode delta frames efficiently
- New clients start from latest keyframe

### 2. **Session Recording & Playback**
- Record all frames to disk
- Seek to keyframes for fast scrubbing
- Replay deterministically from any keyframe

### 3. **Network Synchronization**
- Detect desyncs via state hashes
- Resync clients using nearest keyframe
- Handle packet loss gracefully

### 4. **Deterministic Replay**
- Store complete frame history
- Replay from any frame
- Debug by stepping through frames

---

## Frame Format Notes

### Protocol Buffer Text Format (.pbtxt)
These examples use **Protocol Buffer Text Format** for human readability. In production:
- Use **binary protobuf** for efficiency (`.pb` files)
- Stream over gRPC for real-time updates
- Store compressed for recordings

### Converting to Binary
```bash
# Using protoc compiler
protoc --encode=cryo.frame.Frame cryo_frame.proto < frame_000.frame.pbtxt > frame_000.pb

# Using protoc to decode
protoc --decode=cryo.frame.Frame cryo_frame.proto < frame_000.pb
```

---

## Integration Examples

### Loading a Frame Sequence (C++)
```cpp
#include "cryo_frame.pb.h"
#include <google/protobuf/text_format.h>
#include <fstream>

cryo::frame::Frame loadFrame(const std::string& path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    cryo::frame::Frame frame;
    google::protobuf::TextFormat::ParseFromString(content, &frame);
    return frame;
}
```

### Processing a Frame Stream
```cpp
void processFrameSequence() {
    auto frame0 = loadFrame("frame_000.frame.pbtxt");
    auto frame1 = loadFrame("frame_001.frame.pbtxt");

    // Apply full state from keyframe
    if (frame0.type() == cryo::frame::Frame::FULL) {
        applyFullState(frame0);
    }

    // Apply delta updates
    if (frame1.type() == cryo::frame::Frame::DELTA) {
        applyDeltaUpdate(frame1, frame0);
    }
}
```

---

## Frame Validation

Each frame should be validated before processing:

1. **Frame number continuity** - No gaps in sequence
2. **Timestamp monotonicity** - Time always increases
3. **Reference frame availability** - All referenced frames exist
4. **Message group integrity** - Atomic groups are complete
5. **Compression integrity** - Decompression succeeds

---

## Advanced Topics

### State Hash Verification
Frames can include state hashes for detecting desyncs:
```protobuf
metadata {
  custom_metadata {
    key: "state_hash"
    value: "sha256:abc123..."
  }
}
```

### Bidirectional Frames (B-Frames)
For even higher compression in recordings:
```protobuf
type: BIDIRECTIONAL
reference_frames: [2, 4]  # References frames before AND after
```

### Frame Dropping for Congestion Control
```protobuf
type: SKIP
# Empty frame - signals "no update this frame"
```

---

## See Also

- **Protocol Definition:** `../proto/sync/cryo_frame.proto`
- **Service Definition:** `FrameService` gRPC interface
- **Compression Options:** ZLIB, ZSTD, LZ4, Brotli
- **Message Groups:** Atomic and ordered execution

---

**Generated by:** Cryo Protocol Examples
**Version:** 1.0.0
**Date:** 2025-01-01
