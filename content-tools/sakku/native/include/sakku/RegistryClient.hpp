#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace sakku {

struct LibraryMetadata {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::vector<std::string> tags;
    std::string downloadUrl;
    std::string manifestUrl;
    size_t downloadCount;
    std::string createdAt;
    std::string updatedAt;
};

struct SearchResult {
    std::vector<LibraryMetadata> libraries;
    int totalCount;
    int page;
    int pageSize;
};

class RegistryClient {
public:
    explicit RegistryClient(const std::string& registryUrl);
    ~RegistryClient();

    // Search for libraries in the registry
    std::optional<SearchResult> search(const std::string& query, int page = 1, int pageSize = 10);

    // Get specific library version metadata
    std::optional<LibraryMetadata> getLibrary(const std::string& name, const std::string& version);

    // Download library manifest
    std::optional<std::string> downloadManifest(const std::string& name, const std::string& version);

    // Download library package
    bool downloadLibrary(const std::string& name, const std::string& version, const std::string& outputPath);

    // Publish library to registry
    bool publishLibrary(const std::string& libraryPath, const std::string& apiToken);

    // Get latest version of a library
    std::optional<std::string> getLatestVersion(const std::string& name);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace sakku