#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace sakku {

struct LibraryManifest {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::vector<std::string> tags;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> metadata;

    struct AssetInfo {
        std::string path;
        std::string type;
        size_t size;
        std::string hash;
    };
    std::vector<AssetInfo> assets;
};

class ManifestManager {
public:
    // Create a new manifest from a library directory
    static std::optional<LibraryManifest> createFromDirectory(const std::string& libraryPath);

    // Load manifest from JSON file
    static std::optional<LibraryManifest> loadFromFile(const std::string& manifestPath);

    // Load manifest from JSON string
    static std::optional<LibraryManifest> loadFromString(const std::string& jsonContent);

    // Save manifest to JSON file
    static bool saveToFile(const LibraryManifest& manifest, const std::string& outputPath);

    // Convert manifest to JSON string
    static std::string toJson(const LibraryManifest& manifest);

    // Validate manifest structure
    static bool validate(const LibraryManifest& manifest, std::string& errorMessage);

    // Generate hash for file
    static std::string generateFileHash(const std::string& filePath);
};

} // namespace sakku