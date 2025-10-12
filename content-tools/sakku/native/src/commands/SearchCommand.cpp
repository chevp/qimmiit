#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sakku/RegistryClient.hpp>

namespace sakku {

int searchCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing search query\n";
        std::cerr << "Usage: sakku search <query> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  --page <number>    Page number (default: 1)\n";
        std::cerr << "  --limit <number>   Results per page (default: 10)\n";
        std::cerr << "  --tag <tag>        Filter by tag\n";
        return 1;
    }

    std::string query = args[0];
    std::string registryUrl = "https://registry.sakku.io";
    int page = 1;
    int limit = 10;
    std::string tag;

    // Parse options
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--registry" && i + 1 < args.size()) {
            registryUrl = args[++i];
        } else if (args[i] == "--page" && i + 1 < args.size()) {
            page = std::stoi(args[++i]);
        } else if (args[i] == "--limit" && i + 1 < args.size()) {
            limit = std::stoi(args[++i]);
        } else if (args[i] == "--tag" && i + 1 < args.size()) {
            tag = args[++i];
        }
    }

    std::cout << "Searching for: \"" << query << "\"\n";
    if (!tag.empty()) {
        std::cout << "Tag filter: " << tag << "\n";
    }
    std::cout << "Registry: " << registryUrl << "\n\n";

    // Search registry
    RegistryClient client(registryUrl);
    auto result = client.search(query, page, limit);

    if (!result) {
        std::cerr << "Error: Failed to search registry\n";
        return 1;
    }

    if (result->libraries.empty()) {
        std::cout << "No libraries found matching: \"" << query << "\"\n";
        return 0;
    }

    // Display results
    std::cout << "Found " << result->totalCount << " libraries (showing page "
              << result->page << ")\n";
    std::cout << std::string(80, '=') << "\n\n";

    for (const auto& lib : result->libraries) {
        std::cout << lib.name << " v" << lib.version << "\n";
        std::cout << "  " << lib.description << "\n";
        std::cout << "  Author: " << lib.author << "\n";

        if (!lib.tags.empty()) {
            std::cout << "  Tags: ";
            for (size_t i = 0; i < lib.tags.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << lib.tags[i];
            }
            std::cout << "\n";
        }

        std::cout << "  Downloads: " << lib.downloadCount << "\n";
        std::cout << "\n";
    }

    std::cout << std::string(80, '=') << "\n";
    std::cout << "To install: sakku add <library-name>[@version]\n";

    return 0;
}

} // namespace sakku
