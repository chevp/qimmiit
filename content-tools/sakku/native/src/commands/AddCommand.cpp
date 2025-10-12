#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sakku/RegistryClient.hpp>
#include <sakku/FileUtils.hpp>

namespace sakku {

int addCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing library name\n";
        std::cerr << "Usage: sakku add <library-name>[@version] [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  --registry <url>   Registry URL\n";
        std::cerr << "  --output <path>    Installation directory\n";
        return 1;
    }

    std::string librarySpec = args[0];
    std::string registryUrl = "https://registry.sakku.io";
    std::string outputPath = "./libraries";

    // Parse options
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--registry" && i + 1 < args.size()) {
            registryUrl = args[++i];
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            outputPath = args[++i];
        }
    }

    // Check for registry URL in environment
    const char* envRegistry = std::getenv("SAKKU_REGISTRY_URL");
    if (envRegistry && registryUrl == "https://registry.sakku.io") {
        registryUrl = envRegistry;
    }

    // Check for libraries path in environment
    const char* envPath = std::getenv("SAKKU_LIBRARIES_PATH");
    if (envPath && outputPath == "./libraries") {
        outputPath = envPath;
    }

    // Parse library name and version
    std::string libraryName = librarySpec;
    std::string version = "latest";

    size_t atPos = librarySpec.find('@');
    if (atPos != std::string::npos) {
        libraryName = librarySpec.substr(0, atPos);
        version = librarySpec.substr(atPos + 1);
    }

    std::cout << "Installing: " << libraryName;
    if (version != "latest") {
        std::cout << " v" << version;
    } else {
        std::cout << " (latest)";
    }
    std::cout << "\n";
    std::cout << "Registry: " << registryUrl << "\n";
    std::cout << "Output: " << outputPath << "\n\n";

    // Create output directory
    if (!utils::FileUtils::exists(outputPath)) {
        std::cout << "Creating output directory...\n";
        if (!utils::FileUtils::createDirectory(outputPath)) {
            std::cerr << "Error: Failed to create output directory: " << outputPath << "\n";
            return 1;
        }
    }

    // Get library from registry
    RegistryClient client(registryUrl);

    // Resolve "latest" version
    if (version == "latest") {
        std::cout << "Resolving latest version...\n";
        auto latestVersion = client.getLatestVersion(libraryName);
        if (!latestVersion) {
            std::cerr << "Error: Failed to resolve latest version for: " << libraryName << "\n";
            return 1;
        }
        version = *latestVersion;
        std::cout << "Latest version: " << version << "\n";
    }

    // Get library metadata
    std::cout << "Fetching library metadata...\n";
    auto metadata = client.getLibrary(libraryName, version);
    if (!metadata) {
        std::cerr << "Error: Library not found: " << libraryName << "@" << version << "\n";
        return 1;
    }

    std::cout << "Found: " << metadata->name << " v" << metadata->version << "\n";
    std::cout << "Description: " << metadata->description << "\n";
    std::cout << "Author: " << metadata->author << "\n";

    // Download library
    std::string libraryOutputPath = utils::FileUtils::joinPath(outputPath, libraryName);
    std::cout << "\nDownloading to: " << libraryOutputPath << "\n";

    if (client.downloadLibrary(libraryName, version, libraryOutputPath)) {
        std::cout << "\nSuccess! Library installed successfully.\n";
        std::cout << "Location: " << libraryOutputPath << "\n";
        return 0;
    } else {
        std::cerr << "\nError: Failed to download library\n";
        return 1;
    }
}

} // namespace sakku
