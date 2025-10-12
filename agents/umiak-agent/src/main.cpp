/**
 * Umiak Agent - Scene Manipulation Service
 *
 * A gRPC service that understands scene semantics.
 * NO window. NO orchestration. Just processes gRPC requests.
 */

#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>

#include "UmiakService.hpp"

std::atomic<bool> shutdownRequested(false);

void signalHandler(int signal) {
    std::cout << "\n[Umiak] Shutdown signal received (" << signal << ")\n";
    shutdownRequested = true;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::string serverAddress = "127.0.0.1:50060";
    std::string scenePath;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            serverAddress = "0.0.0.0:" + std::string(argv[++i]);
        } else if (arg == "--scene" && i + 1 < argc) {
            scenePath = argv[++i];
        }
    }

    std::cout << "[Umiak] Starting scene manipulation agent...\n";

    UmiakService umiakService;

    if (!scenePath.empty()) {
        std::cout << "[Umiak] Loading scene: " << scenePath << "\n";
        umiakService.loadScene(scenePath);
    }

    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&umiakService);
    
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
    std::cout << "[Umiak] Listening on " << serverAddress << "\n";
    std::cout << "[Umiak] Press Ctrl+C to shutdown\n";

    while (!shutdownRequested) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Umiak] Shutting down...\n";
    server->Shutdown();
    server->Wait();
    
    return 0;
}
