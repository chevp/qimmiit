/**
 * Sakku - Library Registry CLI
 *
 * Sakku (Inuktitut: "cache" or "storage") is a package manager for game content libraries.
 * Like npm/crates.io for game assets, models, shaders, and other reusable content.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

// Command handlers
namespace sakku {
    int publishCommand(const std::vector<std::string>& args);
    int searchCommand(const std::vector<std::string>& args);
    int addCommand(const std::vector<std::string>& args);
}

void printVersion() {
    std::cout << "Sakku Library Registry CLI v1.0.0\n";
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << "\n";
}

void printHelp() {
    std::cout << R"(
===========================================
  Sakku - Library Registry CLI
  (Inuktitut: "cache" or "storage")
===========================================

A package manager for game content libraries.
Like npm/crates.io for game assets, models, shaders, and reusable content.

USAGE:
    sakku <command> [options]

COMMANDS:
    publish <path>              Publish a library to the registry
    search <query>              Search for libraries in the registry
    add <name>[@version]        Install a library from the registry
    init                        Initialize a new library in current directory
    version                     Show version information
    help                        Show this help message

PUBLISH OPTIONS:
    --registry <url>            Registry URL (default: https://registry.sakku.io)
    --token <token>             API authentication token
    --dry-run                   Validate without publishing

SEARCH OPTIONS:
    --page <number>             Page number (default: 1)
    --limit <number>            Results per page (default: 10)
    --tag <tag>                 Filter by tag

ADD OPTIONS:
    --registry <url>            Registry URL
    --output <path>             Installation directory (default: ./libraries)

EXAMPLES:
    # Publish your library
    sakku publish ./my-library --token YOUR_API_TOKEN

    # Search for libraries
    sakku search "industrial containers"
    sakku search "sci-fi" --tag 3d-models

    # Install from registry
    sakku add industrial-pack@latest
    sakku add sci-fi-assets@1.2.0 --output ./game-assets

    # Initialize new library
    sakku init

REGISTRY API:
    GET  https://registry.sakku.io/api/packages/industrial-pack
    GET  https://registry.sakku.io/api/packages/industrial-pack/1.0.0
    GET  https://cdn.sakku.io/libraries/industrial-pack/v1.0.0/manifest.json

ENVIRONMENT VARIABLES:
    SAKKU_REGISTRY_URL          Default registry URL
    SAKKU_API_TOKEN             API authentication token
    SAKKU_LIBRARIES_PATH        Default library installation path

For more information, visit: https://sakku.io/docs
)";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 0;
    }

    std::string command = argv[1];

    // Version command
    if (command == "version" || command == "--version" || command == "-v") {
        printVersion();
        return 0;
    }

    // Help command
    if (command == "help" || command == "--help" || command == "-h") {
        printHelp();
        return 0;
    }

    // Collect remaining arguments
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    // Command routing
    std::map<std::string, std::function<int(const std::vector<std::string>&)>> commands = {
        {"publish", sakku::publishCommand},
        {"search", sakku::searchCommand},
        {"add", sakku::addCommand}
    };

    auto it = commands.find(command);
    if (it != commands.end()) {
        try {
            return it->second(args);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    // Unknown command
    std::cerr << "Unknown command: " << command << "\n";
    std::cerr << "Run 'sakku help' for usage information.\n";
    return 1;
}