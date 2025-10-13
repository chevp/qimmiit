/**
 * Avannaaq-001 Game Server
 *
 * Main entry point for the game server.
 * Handles initialization, networking, and game loop.
 */

#include <iostream>
#include <memory>
#include <csignal>
#include <thread>
#include <atomic>

#include "game_loop.hpp"
#include "network/connection_manager.hpp"
#include "storage/game_state_store.hpp"

namespace avannaaq {

std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down...\n";
    g_running = false;
}

class GameServer {
public:
    GameServer() = default;
    ~GameServer() = default;

    bool initialize(const std::string& configPath) {
        std::cout << "=============================================================================\n";
        std::cout << "Avannaaq-001 Game Server\n";
        std::cout << "=============================================================================\n\n";

        // TODO: Load configuration from file
        std::cout << "Loading configuration from: " << configPath << "\n";

        // Initialize storage backend (using akutik)
        std::cout << "Initializing storage backend...\n";
        m_storage = std::make_unique<GameStateStore>();
        if (!m_storage->initialize("avannaaq_game.db")) {
            std::cerr << "Failed to initialize storage backend\n";
            return false;
        }

        // Initialize network layer
        std::cout << "Initializing network layer...\n";
        m_connectionManager = std::make_unique<ConnectionManager>();
        if (!m_connectionManager->initialize(8080)) {
            std::cerr << "Failed to initialize network layer\n";
            return false;
        }

        // Initialize game loop
        std::cout << "Initializing game loop...\n";
        m_gameLoop = std::make_unique<GameLoop>();
        if (!m_gameLoop->initialize()) {
            std::cerr << "Failed to initialize game loop\n";
            return false;
        }

        std::cout << "\nServer initialized successfully!\n";
        std::cout << "Listening on port 8080...\n\n";

        return true;
    }

    void run() {
        const double targetFPS = 60.0;
        const double frameTime = 1.0 / targetFPS;

        size_t frameCount = 0;
        auto lastPrint = std::chrono::steady_clock::now();

        while (g_running) {
            auto frameStart = std::chrono::steady_clock::now();

            // Process network events
            m_connectionManager->update();

            // Update game logic
            m_gameLoop->update(frameTime);

            // Save game state periodically
            if (frameCount % 600 == 0) { // Every 10 seconds at 60 FPS
                m_storage->saveGameState();
            }

            // Print status every 5 seconds
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration<double>(now - lastPrint).count() >= 5.0) {
                std::cout << "Server running - Frame: " << frameCount
                          << " | Players: " << m_connectionManager->getPlayerCount()
                          << " | Uptime: " << (frameCount * frameTime) << "s\n";
                lastPrint = now;
            }

            frameCount++;

            // Frame timing
            auto frameEnd = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = frameEnd - frameStart;

            if (elapsed.count() < frameTime) {
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(frameTime - elapsed.count())
                );
            }
        }

        shutdown();
    }

    void shutdown() {
        std::cout << "\nShutting down server...\n";

        if (m_gameLoop) {
            m_gameLoop->shutdown();
        }

        if (m_connectionManager) {
            m_connectionManager->shutdown();
        }

        if (m_storage) {
            m_storage->saveGameState();
            m_storage->shutdown();
        }

        std::cout << "Server shutdown complete.\n";
    }

private:
    std::unique_ptr<GameLoop> m_gameLoop;
    std::unique_ptr<ConnectionManager> m_connectionManager;
    std::unique_ptr<GameStateStore> m_storage;
};

} // namespace avannaaq

int main(int argc, char** argv) {
    // Setup signal handlers
    std::signal(SIGINT, avannaaq::signalHandler);
    std::signal(SIGTERM, avannaaq::signalHandler);

    // Parse command line arguments
    std::string configPath = "config/server_dev.json";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --config <path>  Configuration file (default: config/server_dev.json)\n";
            std::cout << "  --help           Show this help message\n";
            return 0;
        }
    }

    try {
        avannaaq::GameServer server;

        if (!server.initialize(configPath)) {
            std::cerr << "Failed to initialize server\n";
            return 1;
        }

        server.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
