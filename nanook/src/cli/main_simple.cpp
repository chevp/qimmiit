/**
 * Cryo Engine CLI - Simplified Test Version
 */

#include "../core/CentralAgent.hpp"
#include "../core/EngineConfig.hpp"
#include "../modules/RendererModule.hpp"
#include "../modules/CacheModule.hpp"
#include "../modules/AssetModule.hpp"
#include "../modules/HttpModule.hpp"
#include "../modules/ComputeModule.hpp"
#include <iostream>

using namespace cryo::engine;

int main(int argc, char* argv[]) {
    std::cout << "===========================================\n";
    std::cout << "  Cryo Engine CLI - Version 1.0.0\n";
    std::cout << "  Central Orchestrator Architecture\n";
    std::cout << "===========================================\n\n";

    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "Usage: nanook-cli [options] [command]\n\n";
        std::cout << "Options:\n";
        std::cout << "  --config <file>  Load configuration from INI file\n\n";
        std::cout << "Commands:\n";
        std::cout << "  --help, -h       Show this help\n";
        std::cout << "  --version, -v    Show version\n";
        std::cout << "  test             Run basic module test (default)\n";
        return 0;
    }

    if (argc > 1 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
        std::cout << "Nanook Engine CLI v1.0.0\n";
        std::cout << "Build: " << __DATE__ << " " << __TIME__ << "\n";
        return 0;
    }

    // Parse command line arguments
    std::string configFile;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        }
    }

    // Initialize engine
    std::cout << "Initializing Nanook Engine...\n";
    if (!configFile.empty()) {
        std::cout << "Using config file: " << configFile << "\n\n";
    } else {
        std::cout << "Using default in-memory configuration\n\n";
    }

    auto& agent = CentralAgent::getInstance();

    EngineConfig config;
    config.configFile = configFile;  // Will be empty string if not specified
    config.logLevel = "INFO";
    config.threadCount = 4;
    config.memoryPoolSizeMB = 256;

    // Register modules
    agent.registerModule(std::make_shared<CacheModule>());
    agent.registerModule(std::make_shared<AssetModule>());
    agent.registerModule(std::make_shared<RendererModule>());
    agent.registerModule(std::make_shared<HttpModule>());
    agent.registerModule(std::make_shared<ComputeModule>());

    if (!agent.initialize(config)) {
        std::cerr << "ERROR: Failed to initialize engine\n";
        return 1;
    }

    std::cout << "\nEngine initialized successfully!\n";
    std::cout << "Registered modules: " << agent.getModuleCount() << "\n\n";

    // List modules
    auto moduleNames = agent.getModuleNames();
    std::cout << "Modules:\n";
    for (const auto& name : moduleNames) {
        auto module = agent.getModuleByName(name);
        if (module) {
            auto spec = module->getSpecification();
            std::cout << "  - " << name << " [" << moduleTypeToString(spec.type) << "]\n";
        }
    }

    std::cout << "\nShutting down...\n";
    agent.shutdown();

    std::cout << "\nNanook Engine CLI test complete!\n";
    return 0;
}
