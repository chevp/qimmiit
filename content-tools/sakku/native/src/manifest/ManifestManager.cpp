#include <sakku/ManifestManager.hpp>
#include <sakku/FileUtils.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

namespace sakku {

std::optional<LibraryManifest> ManifestManager::createFromDirectory(const std::string& libraryPath) {
    // Look for manifest files
    std::string manifestPath;
    std::vector<std::string> possibleNames = {"sakku.json", "manifest.json", "package.json"};

    for (const auto& name : possibleNames) {
        std::string path = utils::FileUtils::joinPath(libraryPath, name);
        if (utils::FileUtils::exists(path)) {
            manifestPath = path;
            break;
        }
    }

    if (manifestPath.empty()) {
        std::cerr << "No manifest file found (looking for: sakku.json, manifest.json, package.json)\n";
        return std::nullopt;
    }

    return loadFromFile(manifestPath);
}

std::optional<LibraryManifest> ManifestManager::loadFromFile(const std::string& manifestPath) {
    auto content = utils::FileUtils::readFile(manifestPath);
    if (!content) {
        std::cerr << "Failed to read manifest file: " << manifestPath << "\n";
        return std::nullopt;
    }

    return loadFromString(*content);
}

std::optional<LibraryManifest> ManifestManager::loadFromString(const std::string& jsonContent) {
    try {
        json j = json::parse(jsonContent);

        LibraryManifest manifest;
        manifest.name = j["name"];
        manifest.version = j["version"];
        manifest.description = j.value("description", "");
        manifest.author = j.value("author", "");

        if (j.contains("tags")) {
            manifest.tags = j["tags"].get<std::vector<std::string>>();
        }

        if (j.contains("dependencies")) {
            manifest.dependencies = j["dependencies"].get<std::vector<std::string>>();
        }

        if (j.contains("metadata")) {
            manifest.metadata = j["metadata"].get<std::map<std::string, std::string>>();
        }

        if (j.contains("assets")) {
            for (const auto& assetJson : j["assets"]) {
                LibraryManifest::AssetInfo asset;
                asset.path = assetJson["path"];
                asset.type = assetJson.value("type", "unknown");
                asset.size = assetJson.value("size", 0);
                asset.hash = assetJson.value("hash", "");
                manifest.assets.push_back(asset);
            }
        }

        return manifest;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }
}

bool ManifestManager::saveToFile(const LibraryManifest& manifest, const std::string& outputPath) {
    std::string jsonStr = toJson(manifest);
    return utils::FileUtils::writeFile(outputPath, jsonStr);
}

std::string ManifestManager::toJson(const LibraryManifest& manifest) {
    json j;
    j["name"] = manifest.name;
    j["version"] = manifest.version;
    j["description"] = manifest.description;
    j["author"] = manifest.author;

    if (!manifest.tags.empty()) {
        j["tags"] = manifest.tags;
    }

    if (!manifest.dependencies.empty()) {
        j["dependencies"] = manifest.dependencies;
    }

    if (!manifest.metadata.empty()) {
        j["metadata"] = manifest.metadata;
    }

    if (!manifest.assets.empty()) {
        json assetsJson = json::array();
        for (const auto& asset : manifest.assets) {
            json assetJson;
            assetJson["path"] = asset.path;
            assetJson["type"] = asset.type;
            assetJson["size"] = asset.size;
            assetJson["hash"] = asset.hash;
            assetsJson.push_back(assetJson);
        }
        j["assets"] = assetsJson;
    }

    return j.dump(2); // Pretty print with 2-space indent
}

bool ManifestManager::validate(const LibraryManifest& manifest, std::string& errorMessage) {
    if (manifest.name.empty()) {
        errorMessage = "Name is required";
        return false;
    }

    if (manifest.version.empty()) {
        errorMessage = "Version is required";
        return false;
    }

    // Validate version format (simple semantic versioning check)
    size_t dotCount = 0;
    for (char c : manifest.version) {
        if (c == '.') dotCount++;
        else if (!isdigit(c) && c != '-' && c != '+') {
            errorMessage = "Invalid version format (use semantic versioning: x.y.z)";
            return false;
        }
    }

    if (dotCount < 2) {
        errorMessage = "Version must have at least 3 parts (e.g., 1.0.0)";
        return false;
    }

    return true;
}

std::string ManifestManager::generateFileHash(const std::string& filePath) {
    // Simple hash implementation (for real use, implement SHA256)
    auto content = utils::FileUtils::readFile(filePath);
    if (!content) return "";

    std::hash<std::string> hasher;
    size_t hash = hasher(*content);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return oss.str();
}

} // namespace sakku
