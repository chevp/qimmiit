/**
 * @file Nanook.cpp
 * @brief Implementation of simple Nanook wrapper
 */

#include "nanook/Nanook.hpp"
#include "core/CentralAgent.hpp"
#include "core/EngineConfig.hpp"
#include "modules/RendererModule.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace nanook {

// PIMPL implementation
class Nanook::Impl {
public:
    cryo::engine::CentralAgent* centralAgent;
    GLFWwindow* window;
    bool running;

    Impl() : centralAgent(nullptr), window(nullptr), running(false) {
        centralAgent = &cryo::engine::CentralAgent::getInstance();
    }
};

Nanook::Nanook() : impl_(new Impl()) {
}

Nanook::~Nanook() {
    shutdown();
    delete impl_;
}

bool Nanook::initialize(GLFWwindow* window, int argc, char* argv[]) {
    std::cout << "Nanook: Initializing orchestrator...\n";

    impl_->window = window;

    // Create engine configuration
    cryo::engine::EngineConfig config;
    config.window = window;  // Pass window to CentralAgent
    config.logLevel = "INFO";
    config.threadCount = 4;
    config.memoryPoolSizeMB = 512;

    // Parse command line arguments for config overrides
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--config" && i + 1 < argc) {
            config.configFile = argv[++i];
        } else if (arg == "--log-level" && i + 1 < argc) {
            config.logLevel = argv[++i];
        } else if (arg == "--threads" && i + 1 < argc) {
            config.threadCount = std::atoi(argv[++i]);
        }
    }

    // Validate configuration
    if (!config.isValid()) {
        std::cerr << "Nanook: Invalid engine configuration\n";
        return false;
    }

    // Initialize CentralAgent (now with window handle)
    if (!impl_->centralAgent->initialize(config)) {
        std::cerr << "Nanook: Failed to initialize CentralAgent\n";
        return false;
    }

    // Register default modules
    impl_->centralAgent->registerModule(std::make_shared<cryo::engine::RendererModule>());
    // impl_->centralAgent->registerModule(std::make_shared<AssetModule>());
    // impl_->centralAgent->registerModule(std::make_shared<CacheModule>());

    impl_->running = true;
    std::cout << "Nanook: Initialized successfully\n";
    std::cout << "  Modules registered: " << impl_->centralAgent->getModuleCount() << "\n";

    return true;
}

void Nanook::run() {
    std::cout << "Nanook: Starting main loop\n";

    double lastTime = glfwGetTime();
    int frameCount = 0;
    double lastFPSTime = lastTime;

    while (impl_->running && !glfwWindowShouldClose(impl_->window)) {
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // FPS counter
        frameCount++;
        if (currentTime - lastFPSTime >= 1.0) {
            std::cout << "Nanook: FPS: " << frameCount << "\n";
            frameCount = 0;
            lastFPSTime = currentTime;
        }

        // Poll events
        glfwPollEvents();

        // Update all modules through CentralAgent
        impl_->centralAgent->update(deltaTime);

        // Check if shutdown was requested
        if (!impl_->centralAgent->isRunning()) {
            impl_->running = false;
        }
    }

    std::cout << "Nanook: Main loop ended\n";
}

void Nanook::shutdown() {
    if (!impl_->running) return;

    std::cout << "Nanook: Shutting down orchestrator...\n";

    impl_->centralAgent->shutdown();
    impl_->running = false;

    std::cout << "Nanook: Shutdown complete\n";
}

bool Nanook::isRunning() const {
    return impl_->running;
}

} // namespace nanook
