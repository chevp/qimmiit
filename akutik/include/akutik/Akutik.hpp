#pragma once

/**
 * @file Akutik.hpp
 * @brief Main header for the Akutik tool collection
 *
 * "Akutik" (Inuktitut: "storage" / "to store")
 *
 * Akutik is a collection of specialized, reusable tools for data persistence
 * and storage operations. Each tool serves a specific purpose and has no
 * business logic - they are pure utilities that can be used by agents
 * (like Nanook) or other modules.
 *
 * Tools in Akutik:
 * - Ulu: Data slicing and serialization (cuts data into storable pieces)
 * - Niqi: Data ingestion and caching (feeds/nourishes the system with data)
 * - IgluStore: Structured key-value storage (builds data structures)
 *
 * Architecture:
 * - Tools are stateless or minimal-state utilities
 * - No business logic - pure data operations
 * - Designed to be orchestrated by agents (Nanook)
 * - Thread-safe and reusable
 */

// Core storage backend
#include "Storage.hpp"
#include "StorageBackend.hpp"

// Specialized tools
#include "tools/Ulu.hpp"       // Data serialization and slicing
#include "tools/Niqi.hpp"      // Data ingestion and caching
#include "tools/IgluStore.hpp" // Structured storage

namespace akutik {

/**
 * @brief Version information
 */
constexpr const char* VERSION = "1.0.0";

/**
 * @brief Get Akutik library version
 */
inline const char* getVersion() {
    return VERSION;
}

} // namespace akutik
