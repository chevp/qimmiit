/**
 * Umiak Agent - Scene Manipulation Agent
 *
 * Thread-based agent that processes commands asynchronously.
 * Uses C++ structs for communication (no protobuf).
 */

#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include "UmiakAgent.hpp"
#include "AgentCommands.hpp"

std::atomic<bool> shutdownRequested(false);

void signalHandler(int signal) {
    std::cout << "\n[Umiak] Shutdown signal received (" << signal << ")\n";
    shutdownRequested = true;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "[Umiak] Starting scene manipulation agent...\n";

    // Create and start agent
    cryo::agents::UmiakAgent agent;
    agent.start();

    std::cout << "[Umiak] Agent running. Press Ctrl+C to shutdown\n";
    std::cout << "[Umiak] Send commands via AgentCommand C++ structs\n";

    // Example usage demonstration
    std::cout << "\n[Umiak] Running demo commands...\n";

    // Demo 1: Find suzanne-in-box
    {
        cryo::agents::FindEntityRequest findCmd;
        findCmd.name_pattern = "suzanne-in-box";
        agent.sendCommand(findCmd);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Demo 2: Move suzanne-in-box to [2, 2, 2]
    {
        cryo::agents::MoveEntityRequest moveCmd;
        moveCmd.entity_id = "entity_001";  // suzanne-in-box
        moveCmd.x = 2.0f;
        moveCmd.y = 2.0f;
        moveCmd.z = 2.0f;
        moveCmd.relative = false;
        agent.sendCommand(moveCmd);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Demo 3: Move relative
    {
        cryo::agents::MoveEntityRequest moveCmd;
        moveCmd.entity_id = "entity_001";
        moveCmd.x = 1.0f;
        moveCmd.y = 0.0f;
        moveCmd.z = 0.0f;
        moveCmd.relative = true;
        agent.sendCommand(moveCmd);
    }

    std::cout << "\n[Umiak] Demo commands sent. Waiting for shutdown...\n";

    // Main loop
    while (!shutdownRequested) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Umiak] Shutting down...\n";
    agent.stop();

    return 0;
}
