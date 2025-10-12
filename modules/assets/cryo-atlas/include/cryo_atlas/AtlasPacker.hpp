// ==============================================================================
// AtlasPacker.hpp - Texture Atlas Packing System
// Copyright (c) 2025 Patrice Chevillat
// ==============================================================================

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>

namespace cryo::atlas {

// ==============================================================================
// Atlas Sprite Definition
// ==============================================================================

struct AtlasSprite {
    // Identity
    std::string sprite_id;
    std::string source_file;  // Path to source PNG

    // Position in atlas (filled by packer)
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    // Original size (before packing)
    uint32_t original_width = 0;
    uint32_t original_height = 0;

    // UV coordinates (normalized 0-1)
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;

    // Animation data (optional)
    struct AnimationData {
        uint32_t frame_count = 1;
        float frame_duration_ms = 100.0f;
        bool loop = true;
        std::string animation_name;
    };
    AnimationData animation;

    // Metadata
    std::map<std::string, std::string> tags;
    nlohmann::json custom_metadata;

    // Packing hints
    bool rotatable = false;  // Can be rotated 90 degrees
    uint32_t padding = 0;    // Per-sprite padding override

    // Load from JSON
    static AtlasSprite fromJSON(const nlohmann::json& j);

    // Convert to JSON
    nlohmann::json toJSON() const;
};

// ==============================================================================
// Atlas Definition
// ==============================================================================

struct AtlasDefinition {
    std::string atlas_id;
    std::string output_path;

    // Atlas size
    uint32_t width = 4096;
    uint32_t height = 4096;
    bool power_of_two = true;          // Force power-of-2 dimensions
    bool allow_rotation = false;       // Allow sprite rotation for better packing

    // Padding
    uint32_t padding = 2;              // Border between sprites (prevents bleeding)
    uint32_t border = 0;               // Border around atlas edges

    // Input sprites
    std::vector<AtlasSprite> sprites;

    // Auto-discovery
    struct AutoDiscovery {
        bool enabled = false;
        std::string input_directory;
        std::string file_pattern = "*.png";  // Glob pattern
        bool recursive = false;
    };
    AutoDiscovery auto_discovery;

    // Packing algorithm
    enum class PackingAlgorithm {
        MaxRects,         // Maximal Rectangles (best fit)
        Guillotine,       // Guillotine algorithm (fast)
        Skyline,          // Skyline bottom-left
        BestFit           // Best fit decreasing height
    };
    PackingAlgorithm algorithm = PackingAlgorithm::MaxRects;

    // Output options
    enum class ImageFormat {
        PNG,
        KTX2,    // Compressed texture format
        DDS      // DirectDraw Surface
    };
    ImageFormat output_format = ImageFormat::PNG;

    bool premultiply_alpha = false;
    uint32_t compression_quality = 90;  // For lossy formats

    // Metadata export
    std::string metadata_output_path;
    enum class MetadataFormat {
        JSON,
        Protobuf,
        XML
    };
    MetadataFormat metadata_format = MetadataFormat::JSON;

    // Load from JSON file
    static AtlasDefinition fromJSON(const std::string& json_path);

    // Save to JSON file
    void toJSON(const std::string& json_path) const;
};

// ==============================================================================
// Atlas Packing Result
// ==============================================================================

struct AtlasPackingResult {
    bool success = false;
    std::string error_message;

    // Output file info
    std::string output_path;
    std::string metadata_path;
    size_t file_size_bytes = 0;

    // Packing statistics
    struct Statistics {
        uint32_t sprites_packed = 0;
        uint32_t sprites_failed = 0;
        uint32_t atlas_width = 0;
        uint32_t atlas_height = 0;
        float packing_efficiency = 0.0f;  // 0.0 to 1.0
        double packing_time_ms = 0.0;
        double image_generation_time_ms = 0.0;
        double total_time_ms = 0.0;
    };
    Statistics statistics;

    // Sprites that failed to pack
    std::vector<std::string> failed_sprites;
};

// ==============================================================================
// Atlas Packer
// ==============================================================================

class AtlasPacker {
public:
    AtlasPacker();
    ~AtlasPacker();

    // =========================================================================
    // Main Packing Operations
    // =========================================================================

    // Pack sprites into atlas
    AtlasPackingResult pack(const AtlasDefinition& definition);

    // Pack from JSON file
    AtlasPackingResult packFromFile(const std::string& json_path);

    // Pack from directory (auto-discovery)
    AtlasPackingResult packDirectory(
        const std::string& input_directory,
        const std::string& output_path,
        uint32_t atlas_width = 4096,
        uint32_t atlas_height = 4096
    );

    // =========================================================================
    // Advanced Operations
    // =========================================================================

    // Pack multiple atlases (if sprites don't fit in one)
    std::vector<AtlasPackingResult> packMultipleAtlases(
        const AtlasDefinition& definition,
        uint32_t max_atlases = 10
    );

    // Optimize atlas size (find smallest power-of-2 that fits all sprites)
    AtlasPackingResult packOptimized(const AtlasDefinition& definition);

    // =========================================================================
    // Utilities
    // =========================================================================

    // Validate definition
    static bool validateDefinition(const AtlasDefinition& def, std::string& error_message);

    // Estimate required atlas size
    static std::pair<uint32_t, uint32_t> estimateAtlasSize(
        const std::vector<AtlasSprite>& sprites,
        uint32_t padding = 2
    );

    // Get packing statistics
    struct Statistics {
        uint32_t atlases_created = 0;
        uint32_t total_sprites_packed = 0;
        double total_packing_time_ms = 0.0;
        size_t total_output_bytes = 0;
    };

    Statistics getStatistics() const { return statistics_; }
    void resetStatistics() { statistics_ = Statistics{}; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    Statistics statistics_;

    // Helper methods
    bool loadSpriteImages(std::vector<AtlasSprite>& sprites);
    bool packRectangles(std::vector<AtlasSprite>& sprites, uint32_t atlas_width, uint32_t atlas_height);
    bool generateAtlasImage(const std::vector<AtlasSprite>& sprites, const std::string& output_path, const AtlasDefinition& def);
    bool exportMetadata(const std::vector<AtlasSprite>& sprites, const std::string& output_path, AtlasDefinition::MetadataFormat format);
};

// ==============================================================================
// Atlas Metadata (for runtime loading)
// ==============================================================================

struct AtlasMetadata {
    std::string atlas_id;
    uint32_t width;
    uint32_t height;

    // Sprite lookup
    std::map<std::string, AtlasSprite> sprites;

    // Load from JSON file
    static AtlasMetadata fromJSON(const std::string& json_path);

    // Get sprite by ID
    const AtlasSprite* getSprite(const std::string& sprite_id) const;

    // Export to various formats
    void toJSON(const std::string& output_path) const;
    void toProtobuf(const std::string& output_path) const;
};

// ==============================================================================
// Convenience Functions
// ==============================================================================

// Quick pack from directory
bool quickPackDirectory(
    const std::string& input_directory,
    const std::string& output_atlas_path,
    const std::string& output_metadata_path
);

// Generate atlas from sprite sheet (reverse operation)
bool extractSpritesFromSheet(
    const std::string& sprite_sheet_path,
    const std::string& output_directory,
    uint32_t sprite_width,
    uint32_t sprite_height
);

} // namespace cryo::atlas