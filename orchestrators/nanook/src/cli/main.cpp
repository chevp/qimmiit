/**
 * Cryo Engine CLI - Command Line Interface
 *
 * Provides command-line tools for Cryo Engine, including:
 * - Specification generation (spec)
 * - Configuration validation (config validate)
 * - Module introspection (modules list/info)
 * - Version info (--version)
 * - Help (--help)
 */

#include "../core/CentralAgent.hpp"
#include "../core/SpecificationGenerator.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace cryo::engine;

/**
 * Command line arguments
 */
struct CLIArgs {
    std::string command;  // "spec", "config", "modules", etc.
    std::map<std::string, std::string> options;
    std::vector<std::string> positionalArgs;
};

/**
 * Parse command line arguments
 */
CLIArgs parseArgs(int argc, char* argv[]) {
    CLIArgs args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            args.command = "help";
        } else if (arg == "--version" || arg == "-v") {
            args.command = "version";
        } else if (arg == "spec") {
            args.command = "spec";
        } else if (arg == "config") {
            args.command = "config";
            if (i + 1 < argc) {
                args.positionalArgs.push_back(argv[++i]);  // "validate", etc.
            }
        } else if (arg == "modules") {
            args.command = "modules";
            if (i + 1 < argc) {
                args.positionalArgs.push_back(argv[++i]);  // "list", "info", etc.
            }
        } else if (arg.rfind("--", 0) == 0) {
            // Option flag (--format, --output, etc.)
            std::string key = arg.substr(2);
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.options[key] = argv[++i];
            } else {
                args.options[key] = "true";  // Boolean flag
            }
        } else {
            args.positionalArgs.push_back(arg);
        }
    }

    return args;
}

/**
 * Show help message
 */
void showHelp() {
    std::cout << R"(
Cryo Engine CLI - Command Line Interface

USAGE:
    cryo-engine-cli [COMMAND] [OPTIONS]

COMMANDS:
    spec                Generate engine specification
        --format <fmt>      Output format: markdown|json|yaml|html (default: markdown)
        --output <file>     Output file (default: stdout)
        --module <name>     Generate spec for specific module only
        --section <name>    Generate specific section: modules|events|config|api|all

    config validate     Validate configuration file
        --config <file>     Config file to validate (default: arctic.config.xml)

    modules list        List all registered modules
    modules info <name> Show detailed module information

    --version, -v       Show version information
    --help, -h          Show this help message

EXAMPLES:
    # Generate full spec (markdown to stdout)
    cryo-engine-cli spec

    # Generate JSON spec to file
    cryo-engine-cli spec --format json --output spec.json

    # Generate spec for specific module
    cryo-engine-cli spec --module RendererModule

    # Generate only configuration section
    cryo-engine-cli spec --section config

    # Validate config file
    cryo-engine-cli config validate --config arctic.config.xml

    # List all modules
    cryo-engine-cli modules list

    # Show module info
    cryo-engine-cli modules info RendererModule

    # Show version
    cryo-engine-cli --version
)";
}

/**
 * Show version information
 */
void showVersion() {
    std::cout << "Cryo Engine CLI\n";
    std::cout << "Version: 1.0.0\n";
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << "\n";
    std::cout << "Platform: ";
#ifdef _WIN32
    std::cout << "Windows\n";
#elif __linux__
    std::cout << "Linux\n";
#elif __APPLE__
    std::cout << "macOS\n";
#else
    std::cout << "Unknown\n";
#endif
}

/**
 * Generate specification
 */
int cmdSpec(const CLIArgs& args) {
    // Get format
    std::string format = "markdown";
    if (args.options.count("format")) {
        format = args.options.at("format");
    }

    // Get output file (empty = stdout)
    std::string outputFile;
    if (args.options.count("output")) {
        outputFile = args.options.at("output");
    }

    // Get specific module (empty = all modules)
    std::string moduleFilter;
    if (args.options.count("module")) {
        moduleFilter = args.options.at("module");
    }

    // Get specific section (empty = all sections)
    std::string sectionFilter;
    if (args.options.count("section")) {
        sectionFilter = args.options.at("section");
    }

    // Initialize engine (minimal initialization for spec generation)
    auto& agent = CentralAgent::getInstance();

    EngineConfig config;
    config.configFile = "arctic.config.xml";
    config.logLevel = "INFO";
    config.threadCount = 4;
    config.memoryPoolSizeMB = 512;

    if (!agent.initialize(config)) {
        std::cerr << "ERROR: Failed to initialize engine\n";
        return 1;
    }

    // Generate specification
    SpecFormat specFormat = SpecFormat::MARKDOWN;
    if (format == "json") {
        specFormat = SpecFormat::JSON;
    } else if (format == "yaml") {
        specFormat = SpecFormat::YAML;
    } else if (format == "html") {
        specFormat = SpecFormat::HTML;
    }

    agent.generateSpecification(specFormat, outputFile, moduleFilter, sectionFilter);

    agent.shutdown();

    if (!outputFile.empty()) {
        std::cout << "Specification written to: " << outputFile << "\n";
    }

    return 0;
}

/**
 * Validate configuration
 */
int cmdConfigValidate(const CLIArgs& args) {
    std::string configFile = "arctic.config.xml";
    if (args.options.count("config")) {
        configFile = args.options.at("config");
    }

    std::cout << "Validating configuration: " << configFile << "\n";

    // Initialize config manager
    ConfigManager configManager;
    if (!configManager.initialize(configFile)) {
        std::cerr << "ERROR: Failed to load configuration file\n";
        return 1;
    }

    // Validate configuration
    // (ConfigManager already validates on load)

    std::cout << "✓ Configuration is valid\n";
    std::cout << "  Sections: " << configManager.getSectionCount() << "\n";
    std::cout << "  Options: " << configManager.getOptionCount() << "\n";

    return 0;
}

/**
 * List all modules
 */
int cmdModulesList(const CLIArgs& args) {
    // Initialize engine
    auto& agent = CentralAgent::getInstance();

    EngineConfig config;
    config.configFile = "arctic.config.xml";
    config.logLevel = "WARN";
    config.threadCount = 4;
    config.memoryPoolSizeMB = 512;

    if (!agent.initialize(config)) {
        std::cerr << "ERROR: Failed to initialize engine\n";
        return 1;
    }

    // List modules
    auto moduleNames = agent.getModuleNames();

    std::cout << "Registered Modules (" << moduleNames.size() << "):\n";
    std::cout << "=====================================\n";

    for (const auto& name : moduleNames) {
        // Get module spec for type info
        auto module = agent.getModuleByName(name);
        if (module) {
            auto spec = module->getSpecification();
            std::cout << "  - " << name;
            std::cout << " [" << moduleTypeToString(spec.type) << "]";
            std::cout << " v" << spec.version << "\n";
            std::cout << "    " << spec.description << "\n";
        }
    }

    agent.shutdown();
    return 0;
}

/**
 * Show module info
 */
int cmdModulesInfo(const CLIArgs& args) {
    if (args.positionalArgs.size() < 2) {
        std::cerr << "ERROR: Module name required\n";
        std::cerr << "Usage: cryo-engine-cli modules info <module-name>\n";
        return 1;
    }

    std::string moduleName = args.positionalArgs[1];

    // Initialize engine
    auto& agent = CentralAgent::getInstance();

    EngineConfig config;
    config.configFile = "arctic.config.xml";
    config.logLevel = "WARN";
    config.threadCount = 4;
    config.memoryPoolSizeMB = 512;

    if (!agent.initialize(config)) {
        std::cerr << "ERROR: Failed to initialize engine\n";
        return 1;
    }

    // Get module
    auto module = agent.getModuleByName(moduleName);
    if (!module) {
        std::cerr << "ERROR: Module not found: " << moduleName << "\n";
        agent.shutdown();
        return 1;
    }

    // Get and display specification
    auto spec = module->getSpecification();

    std::cout << "Module: " << spec.name << "\n";
    std::cout << "=====================================\n";
    std::cout << "Version: " << spec.version << "\n";
    std::cout << "Type: " << moduleTypeToString(spec.type) << "\n";
    std::cout << "Description: " << spec.description << "\n";
    std::cout << "\n";

    if (!spec.dependencies.empty()) {
        std::cout << "Dependencies:\n";
        for (const auto& dep : spec.dependencies) {
            std::cout << "  - " << dep << "\n";
        }
        std::cout << "\n";
    }

    if (!spec.publicMethods.empty()) {
        std::cout << "Public API (" << spec.publicMethods.size() << " methods):\n";
        for (const auto& method : spec.publicMethods) {
            std::cout << "  - " << method.name << "(";
            for (size_t i = 0; i < method.parameters.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << method.parameters[i].name << ": " << method.parameters[i].type;
            }
            std::cout << ") -> " << method.returnType << "\n";
            std::cout << "    " << method.description << "\n";
        }
        std::cout << "\n";
    }

    if (!spec.configOptions.empty()) {
        std::cout << "Configuration Options (" << spec.configOptions.size() << "):\n";
        for (const auto& opt : spec.configOptions) {
            std::cout << "  - " << opt.key << " (" << opt.type << ")\n";
            std::cout << "    " << opt.description << "\n";
            std::cout << "    Default: " << opt.defaultValue;
            std::cout << ", Current: " << opt.currentValue << "\n";
        }
        std::cout << "\n";
    }

    if (!spec.publishedEvents.empty()) {
        std::cout << "Published Events:\n";
        for (const auto& event : spec.publishedEvents) {
            std::cout << "  - " << event << "\n";
        }
        std::cout << "\n";
    }

    if (!spec.subscribedEvents.empty()) {
        std::cout << "Subscribed Events:\n";
        for (const auto& event : spec.subscribedEvents) {
            std::cout << "  - " << event << "\n";
        }
        std::cout << "\n";
    }

    if (!spec.statistics.empty()) {
        std::cout << "Runtime Statistics:\n";
        for (const auto& [key, value] : spec.statistics) {
            std::cout << "  - " << key << ": " << value << "\n";
        }
    }

    agent.shutdown();
    return 0;
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        showHelp();
        return 0;
    }

    CLIArgs args = parseArgs(argc, argv);

    if (args.command == "help") {
        showHelp();
        return 0;
    }

    if (args.command == "version") {
        showVersion();
        return 0;
    }

    if (args.command == "spec") {
        return cmdSpec(args);
    }

    if (args.command == "config") {
        if (!args.positionalArgs.empty() && args.positionalArgs[0] == "validate") {
            return cmdConfigValidate(args);
        } else {
            std::cerr << "ERROR: Unknown config subcommand\n";
            std::cerr << "Usage: cryo-engine-cli config validate [--config <file>]\n";
            return 1;
        }
    }

    if (args.command == "modules") {
        if (!args.positionalArgs.empty()) {
            if (args.positionalArgs[0] == "list") {
                return cmdModulesList(args);
            } else if (args.positionalArgs[0] == "info") {
                return cmdModulesInfo(args);
            }
        }
        std::cerr << "ERROR: Unknown modules subcommand\n";
        std::cerr << "Usage: cryo-engine-cli modules <list|info>\n";
        return 1;
    }

    std::cerr << "ERROR: Unknown command: " << args.command << "\n";
    showHelp();
    return 1;
}