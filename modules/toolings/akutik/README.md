# Akutik

**"Akutik"** (Inuktitut: "to store" / "storage") — A collection of specialized storage and persistence tools for the Qimmiit engine.

## Overview

Akutik is a tool library containing specialized, reusable utilities for data storage and persistence. Each tool serves a specific purpose with **no business logic** - they are pure utilities designed to be orchestrated by agents like **Nanook**.

### Philosophy

Like traditional Inuit tools, each component in Akutik:
- Has a single, well-defined purpose
- Contains no business logic - pure data operations
- Is reusable across different contexts
- Can be combined with other tools for complex operations
- Is designed to be orchestrated by higher-level agents (Nanook)

## Tools

### Ulu - Data Slicing and Serialization
Named after the traditional Inuit crescent knife (ᐅᓗ), Ulu cuts data into storable pieces.

**Capabilities:**
- Serialize/deserialize data to binary format
- Slice large data into chunks
- Merge chunks back together
- Data integrity verification (checksums)

### Niqi - Data Ingestion and Caching
Named after the Inuktitut word for "food/meat" (ᓂᕿ), Niqi nourishes the system with cached data.

**Capabilities:**
- In-memory LRU caching with configurable size limits
- Time-to-live (TTL) expiration
- Cache hit/miss statistics
- Automatic eviction and size management

### IgluStore - Structured Storage
Named after "Iglu" (ᐃᒡᓗ), the traditional shelter, IgluStore builds structured data organization.

**Capabilities:**
- Namespace-based key organization
- Batch operations
- Transaction support (begin/commit/rollback)
- Query helpers for namespace operations

## Architecture

```
┌─────────────────────────────────────────┐
│         Nanook (Orchestrator)           │  ← Business logic lives here
│  - Coordinates tools                    │
│  - Makes decisions                      │
│  - Implements workflows                 │
└─────────────────┬───────────────────────┘
                  │
                  │ uses
                  ▼
┌─────────────────────────────────────────┐
│         Akutik (Tool Library)           │  ← No business logic
│                                         │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐ │
│  │   Ulu   │  │  Niqi   │  │  Iglu   │ │
│  │ (slice) │  │ (cache) │  │ (store) │ │
│  └─────────┘  └─────────┘  └─────────┘ │
│                                         │
│  ┌───────────────────────────────────┐  │
│  │  Storage Backend (SQLite)         │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## Usage

### Using Individual Tools

#### Ulu - Data Slicing
```cpp
#include <akutik/tools/Ulu.hpp>

// Serialize data
std::string text = "Hello, Arctic!";
auto serialized = akutik::tools::Ulu::serialize(text);

// Slice into chunks
auto chunks = akutik::tools::Ulu::slice(serialized, 1024);

// Calculate checksum
uint32_t crc = akutik::tools::Ulu::checksum(serialized);

// Verify integrity
bool valid = akutik::tools::Ulu::verify(serialized, crc);
```

#### Niqi - Caching
```cpp
#include <akutik/tools/Niqi.hpp>

// Create cache (100MB, 1 hour TTL)
akutik::tools::Niqi cache(100 * 1024 * 1024, 3600);

// Store data
std::vector<uint8_t> data = {1, 2, 3, 4, 5};
cache.put("my_key", data);

// Retrieve data
auto result = cache.get("my_key");
if (result) {
    // Use cached data
}

// Get statistics
auto stats = cache.getStatistics();
std::cout << "Hits: " << stats.hits << ", Misses: " << stats.misses << std::endl;
```

#### IgluStore - Structured Storage
```cpp
#include <akutik/tools/IgluStore.hpp>

auto storage = std::make_shared<akutik::Storage>("game.db");
akutik::tools::IgluStore store(storage);

// Store in namespaces
store.put("player", "name", std::string("Nanook"));
store.put("player", "level", 42);
store.put("settings", "volume", 0.8f);

// Retrieve
auto name = store.get<std::string>("player", "name");
auto level = store.get<int>("player", "level");

// List keys in namespace
auto playerKeys = store.keysInNamespace("player");

// Batch operations with transaction
store.batch([&]() {
    store.put("inventory", "coins", 100);
    store.put("inventory", "gems", 50);
    return true; // Commit
});
```

### Combined Usage (Orchestrated by Nanook)

```cpp
// In Nanook module - this is where business logic lives
class DataPersistenceWorkflow {
public:
    void savePlayerData(const PlayerData& data) {
        // Business logic: decide what to save and how

        // Use Ulu to serialize
        auto serialized = akutik::tools::Ulu::serialize(data.score);

        // Use Niqi to cache
        cache_.put("player:score", serialized);

        // Use IgluStore for persistent storage
        store_.put("player", "score", data.score);
        store_.put("player", "timestamp", getCurrentTime());

        log("Player data saved successfully");
    }

private:
    akutik::tools::Niqi cache_;
    akutik::tools::IgluStore store_;
};
```

## Features

- **Specialized Tools**: Each tool has a specific, well-defined purpose
- **No Business Logic**: Pure data operations only - orchestration is external
- **Type-safe API**: Template-based interface for different data types
- **Multiple Storage Backends**: SQLite (implemented), File, Memory (planned)
- **Thread-safe**: All tools can be used safely from multiple threads
- **Composable**: Tools can be combined for complex workflows

## Building

Akutik is built as part of the Qimmiit project:

```bash
cmake --preset default
cmake --build build
```

## Dependencies

- SQLite3
- C++17 compatible compiler

## Integration

To use Akutik in your module:

```cmake
target_link_libraries(your_module PRIVATE akutik)
```

```cpp
#include <akutik/Akutik.hpp>
```

## Design Principles

1. **Tools, not Frameworks**: Akutik provides tools, not a framework
2. **No Hidden State**: Minimal internal state, explicit parameters
3. **Composability**: Tools can be combined freely
4. **Agent-Orchestrated**: Designed to be coordinated by agents (Nanook)
5. **Single Responsibility**: Each tool does one thing well

## License

[License information needed]
