#include <sakku/RegistryClient.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace sakku {

// Helper function for CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Helper function for CURL write to file
static size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

class RegistryClient::Impl {
public:
    std::string registryUrl;
    CURL* curl;

    Impl(const std::string& url) : registryUrl(url) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
    }

    ~Impl() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    std::optional<std::string> httpGet(const std::string& url) {
        if (!curl) return std::nullopt;

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            return std::nullopt;
        }

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (httpCode != 200) {
            std::cerr << "HTTP error: " << httpCode << "\n";
            return std::nullopt;
        }

        return response;
    }

    bool httpDownloadFile(const std::string& url, const std::string& outputPath) {
        if (!curl) return false;

        FILE* fp = fopen(outputPath.c_str(), "wb");
        if (!fp) {
            std::cerr << "Failed to open file for writing: " << outputPath << "\n";
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        fclose(fp);

        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            return false;
        }

        return true;
    }

    std::optional<json> httpPostJson(const std::string& url, const json& data, const std::string& token) {
        if (!curl) return std::nullopt;

        std::string jsonStr = data.dump();
        std::string response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!token.empty()) {
            std::string authHeader = "Authorization: Bearer " + token;
            headers = curl_slist_append(headers, authHeader.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            return std::nullopt;
        }

        try {
            return json::parse(response);
        } catch (const json::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << "\n";
            return std::nullopt;
        }
    }
};

RegistryClient::RegistryClient(const std::string& registryUrl)
    : pImpl(std::make_unique<Impl>(registryUrl)) {}

RegistryClient::~RegistryClient() = default;

std::optional<SearchResult> RegistryClient::search(const std::string& query, int page, int pageSize) {
    std::ostringstream url;
    url << pImpl->registryUrl << "/api/search?q=" << query
        << "&page=" << page << "&limit=" << pageSize;

    auto response = pImpl->httpGet(url.str());
    if (!response) return std::nullopt;

    try {
        json j = json::parse(*response);

        SearchResult result;
        result.page = j["page"];
        result.pageSize = j["pageSize"];
        result.totalCount = j["total"];

        for (const auto& item : j["libraries"]) {
            LibraryMetadata lib;
            lib.name = item["name"];
            lib.version = item["version"];
            lib.description = item["description"];
            lib.author = item["author"];
            lib.downloadUrl = item["downloadUrl"];
            lib.manifestUrl = item["manifestUrl"];
            lib.downloadCount = item["downloads"];
            lib.createdAt = item["createdAt"];
            lib.updatedAt = item["updatedAt"];

            if (item.contains("tags")) {
                lib.tags = item["tags"].get<std::vector<std::string>>();
            }

            result.libraries.push_back(lib);
        }

        return result;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }
}

std::optional<LibraryMetadata> RegistryClient::getLibrary(const std::string& name, const std::string& version) {
    std::ostringstream url;
    url << pImpl->registryUrl << "/api/packages/" << name << "/" << version;

    auto response = pImpl->httpGet(url.str());
    if (!response) return std::nullopt;

    try {
        json j = json::parse(*response);

        LibraryMetadata lib;
        lib.name = j["name"];
        lib.version = j["version"];
        lib.description = j["description"];
        lib.author = j["author"];
        lib.downloadUrl = j["downloadUrl"];
        lib.manifestUrl = j["manifestUrl"];
        lib.downloadCount = j["downloads"];
        lib.createdAt = j["createdAt"];
        lib.updatedAt = j["updatedAt"];

        if (j.contains("tags")) {
            lib.tags = j["tags"].get<std::vector<std::string>>();
        }

        return lib;
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }
}

std::optional<std::string> RegistryClient::downloadManifest(const std::string& name, const std::string& version) {
    std::ostringstream url;
    url << pImpl->registryUrl << "/api/packages/" << name << "/" << version << "/manifest";

    return pImpl->httpGet(url.str());
}

bool RegistryClient::downloadLibrary(const std::string& name, const std::string& version, const std::string& outputPath) {
    auto metadata = getLibrary(name, version);
    if (!metadata) return false;

    std::cout << "Downloading from: " << metadata->downloadUrl << "\n";

    std::string archivePath = outputPath + ".zip";
    if (!pImpl->httpDownloadFile(metadata->downloadUrl, archivePath)) {
        return false;
    }

    // TODO: Extract archive
    std::cout << "Downloaded to: " << archivePath << "\n";
    std::cout << "Note: Automatic extraction not yet implemented. Please extract manually.\n";

    return true;
}

bool RegistryClient::publishLibrary(const std::string& libraryPath, const std::string& apiToken) {
    // TODO: Create archive, upload, and publish
    std::cout << "Publishing library from: " << libraryPath << "\n";
    std::cout << "Note: Publishing not yet fully implemented.\n";
    std::cout << "This would:\n";
    std::cout << "  1. Create archive from library directory\n";
    std::cout << "  2. Upload archive to CDN\n";
    std::cout << "  3. Register package with registry\n";

    return false; // Not implemented yet
}

std::optional<std::string> RegistryClient::getLatestVersion(const std::string& name) {
    std::ostringstream url;
    url << pImpl->registryUrl << "/api/packages/" << name << "/latest";

    auto response = pImpl->httpGet(url.str());
    if (!response) return std::nullopt;

    try {
        json j = json::parse(*response);
        return j["version"].get<std::string>();
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }
}

} // namespace sakku
