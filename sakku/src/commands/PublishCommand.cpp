#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sakku/RegistryClient.hpp>
#include <sakku/ManifestManager.hpp>
#include <sakku/FileUtils.hpp>

namespace sakku {

int publishCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing library path\n";
        std::cerr << "Usage: sakku publish <library-path> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  --registry <url>   Registry URL\n";
        std::cerr << "  --token <token>    API authentication token\n";
        std::cerr << "  --dry-run          Validate without publishing\n";
        return 1;
    }

    std::string libraryPath = args[0];
    std::string registryUrl = "https://registry.sakku.io";
    std::string apiToken;
    bool dryRun = false;

    // Parse options
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--registry" && i + 1 < args.size()) {
            registryUrl = args[++i];
        } else if (args[i] == "--token" && i + 1 < args.size()) {
            apiToken = args[++i];
        } else if (args[i] == "--dry-run") {
            dryRun = true;
        }
    }

    // Check for token in environment if not provided
    if (apiToken.empty()) {
        const char* envToken = std::getenv("SAKKU_API_TOKEN");
        if (envToken) {
            apiToken = envToken;
        }
    }

    if (apiToken.empty() && !dryRun) {
        std::cerr << "Error: API token required for publishing\n";
        std::cerr << "Provide via --token option or SAKKU_API_TOKEN environment variable\n";
        return 1;
    }

    std::cout << "Publishing library from: " << libraryPath << "\n";
    std::cout << "Registry: " << registryUrl << "\n";

    // Validate library path
    if (!utils::FileUtils::exists(libraryPath)) {
        std::cerr << "Error: Library path does not exist: " << libraryPath << "\n";
        return 1;
    }

    if (!utils::FileUtils::isDirectory(libraryPath)) {
        std::cerr << "Error: Library path must be a directory: " << libraryPath << "\n";
        return 1;
    }

    // Create or load manifest
    std::cout << "Creating manifest...\n";
    auto manifest = ManifestManager::createFromDirectory(libraryPath);
    if (!manifest) {
        std::cerr << "Error: Failed to create manifest from library\n";
        std::cerr << "Make sure the library contains a manifest.json or sakku.json file\n";
        return 1;
    }

    // Validate manifest
    std::string errorMessage;
    if (!ManifestManager::validate(*manifest, errorMessage)) {
        std::cerr << "Error: Invalid manifest: " << errorMessage << "\n";
        return 1;
    }

    std::cout << "Library: " << manifest->name << " v" << manifest->version << "\n";
    std::cout << "Description: " << manifest->description << "\n";
    std::cout << "Assets: " << manifest->assets.size() << " files\n";

    if (dryRun) {
        std::cout << "\nDry run mode - validation successful!\n";
        std::cout << "Manifest is valid and ready for publishing.\n";
        return 0;
    }

    // Publish to registry
    std::cout << "\nPublishing to registry...\n";
    RegistryClient client(registryUrl);
    if (client.publishLibrary(libraryPath, apiToken)) {
        std::cout << "\nSuccess! Library published successfully.\n";
        std::cout << "View at: " << registryUrl << "/packages/" << manifest->name << "\n";
        return 0;
    } else {
        std::cerr << "\nError: Failed to publish library\n";
        return 1;
    }
}

} // namespace sakku
