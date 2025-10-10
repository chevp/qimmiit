/**
 * Arctic IPC Server - WebSocket-based Inter-Process Communication Server
 *
 * Provides WebSocket server for communication between HTML/JS tools and C++ renderer.
 * Uses localhost WebSocket connection on port 52010.
 *
 * Protocol: JSON-based request/response with event broadcasting
 *
 * Usage:
 *   ArcticIPCServer server(52010);
 *   server.start();
 *
 *   server.onMessage([](const std::string& type, const json& data) {
 *       if (type == "getMaterial") {
 *           return getMaterialData(data["id"]);
 *       }
 *   });
 *
 *   server.broadcast("sceneLoaded", sceneData);
 */

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>

// Forward declarations for WebSocket library (using libwebsockets or similar)
// #include <libwebsockets.h>  // Or websocketpp, uWebSockets, etc.

// JSON library (nlohmann/json recommended)
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace arctic {
namespace ipc {

/**
 * WebSocket connection wrapper
 */
class Connection {
public:
    Connection(void* wsHandle, const std::string& clientId)
        : m_wsHandle(wsHandle)
        , m_clientId(clientId)
        , m_connected(true)
    {}

    void* getHandle() const { return m_wsHandle; }
    const std::string& getClientId() const { return m_clientId; }
    bool isConnected() const { return m_connected; }
    void setConnected(bool connected) { m_connected = connected; }

private:
    void* m_wsHandle;
    std::string m_clientId;
    bool m_connected;
};

/**
 * Message handler callback type
 * Parameters: (type, requestId, data, connection)
 * Returns: JSON response data
 */
using MessageHandler = std::function<json(
    const std::string& type,
    const std::string& requestId,
    const json& data,
    std::shared_ptr<Connection> conn
)>;

/**
 * Arctic IPC WebSocket Server
 */
class ArcticIPCServer {
public:
    /**
     * Constructor
     * @param port WebSocket port (default: 52010)
     * @param host Host address (default: localhost)
     */
    ArcticIPCServer(int port = 52010, const std::string& host = "127.0.0.1");

    /**
     * Destructor - stops server if running
     */
    ~ArcticIPCServer();

    /**
     * Start the WebSocket server
     * @return true if started successfully
     */
    bool start();

    /**
     * Stop the WebSocket server
     */
    void stop();

    /**
     * Check if server is running
     */
    bool isRunning() const { return m_running; }

    /**
     * Register message handler for specific message type
     * @param type Message type (e.g., "getMaterial")
     * @param handler Callback function
     */
    void on(const std::string& type, MessageHandler handler);

    /**
     * Broadcast message to all connected clients
     * @param type Event type
     * @param data Event data
     */
    void broadcast(const std::string& type, const json& data = json::object());

    /**
     * Send message to specific client
     * @param clientId Client identifier
     * @param type Message type
     * @param data Message data
     */
    void send(const std::string& clientId, const std::string& type, const json& data = json::object());

    /**
     * Send response to request
     * @param conn Connection that made request
     * @param requestId Original request ID
     * @param data Response data
     */
    void sendResponse(std::shared_ptr<Connection> conn, const std::string& requestId, const json& data);

    /**
     * Send error response
     * @param conn Connection that made request
     * @param requestId Original request ID
     * @param error Error message
     */
    void sendError(std::shared_ptr<Connection> conn, const std::string& requestId, const std::string& error);

    /**
     * Get number of connected clients
     */
    size_t getClientCount() const;

    /**
     * Get list of connected client IDs
     */
    std::vector<std::string> getClientIds() const;

    /**
     * Set connection callback
     */
    void onConnect(std::function<void(std::shared_ptr<Connection>)> callback);

    /**
     * Set disconnection callback
     */
    void onDisconnect(std::function<void(const std::string& clientId)> callback);

private:
    /**
     * Server thread main loop
     */
    void serverLoop();

    /**
     * Handle incoming message
     */
    void handleMessage(std::shared_ptr<Connection> conn, const std::string& message);

    /**
     * Send raw data to connection
     */
    void sendRaw(std::shared_ptr<Connection> conn, const std::string& message);

    /**
     * Add new connection
     */
    void addConnection(void* wsHandle, const std::string& clientId);

    /**
     * Remove connection
     */
    void removeConnection(const std::string& clientId);

private:
    int m_port;
    std::string m_host;
    bool m_running;

    void* m_wsContext;  // libwebsockets context or equivalent
    std::thread m_serverThread;

    std::mutex m_connectionsMutex;
    std::unordered_map<std::string, std::shared_ptr<Connection>> m_connections;

    std::mutex m_handlersMutex;
    std::unordered_map<std::string, MessageHandler> m_handlers;

    std::function<void(std::shared_ptr<Connection>)> m_onConnectCallback;
    std::function<void(const std::string&)> m_onDisconnectCallback;
};

/**
 * Convenience class for registering default Arctic IPC handlers
 */
class ArcticIPCHandlers {
public:
    /**
     * Register all default handlers for Arctic renderer
     * @param server IPC server instance
     * @param sceneManager Pointer to scene manager
     * @param materialManager Pointer to material manager
     * @param assetManager Pointer to asset manager
     */
    static void registerDefaults(
        ArcticIPCServer& server,
        void* sceneManager = nullptr,
        void* materialManager = nullptr,
        void* assetManager = nullptr
    );

    /**
     * Register scene management handlers
     */
    static void registerSceneHandlers(ArcticIPCServer& server, void* sceneManager);

    /**
     * Register material management handlers
     */
    static void registerMaterialHandlers(ArcticIPCServer& server, void* materialManager);

    /**
     * Register asset management handlers
     */
    static void registerAssetHandlers(ArcticIPCServer& server, void* assetManager);

    /**
     * Register shader management handlers
     */
    static void registerShaderHandlers(ArcticIPCServer& server, void* shaderManager);

    /**
     * Register camera control handlers
     */
    static void registerCameraHandlers(ArcticIPCServer& server, void* cameraController);

    /**
     * Register rendering control handlers
     */
    static void registerRenderHandlers(ArcticIPCServer& server, void* renderer);

    /**
     * Register debug/monitoring handlers
     */
    static void registerDebugHandlers(ArcticIPCServer& server, void* debugInfo);
};

} // namespace ipc
} // namespace arctic

// Example usage in main application:
/*

#include <coregfx/ipc/arctic_ipc_server.hpp>

using namespace arctic::ipc;

int main() {
    // Create IPC server
    ArcticIPCServer ipcServer(52010);

    // Register custom handler
    ipcServer.on("getMaterial", [](const std::string& type, const std::string& reqId,
                                     const json& data, std::shared_ptr<Connection> conn) {
        std::string materialId = data["id"];

        // Fetch material from your material system
        Material* mat = g_materialManager->getMaterial(materialId);

        if (!mat) {
            throw std::runtime_error("Material not found");
        }

        // Return material data as JSON
        return json{
            {"id", mat->id},
            {"name", mat->name},
            {"metallic", mat->metallic},
            {"roughness", mat->roughness},
            {"textures", {
                {"albedo", mat->albedoTexture ? mat->albedoTexture->toBase64() : ""},
                {"normal", mat->normalTexture ? mat->normalTexture->toBase64() : ""}
            }}
        };
    });

    // Register default Arctic handlers
    ArcticIPCHandlers::registerDefaults(
        ipcServer,
        &g_sceneManager,
        &g_materialManager,
        &g_assetManager
    );

    // Set connection callbacks
    ipcServer.onConnect([](std::shared_ptr<Connection> conn) {
        std::cout << "Client connected: " << conn->getClientId() << std::endl;
    });

    ipcServer.onDisconnect([](const std::string& clientId) {
        std::cout << "Client disconnected: " << clientId << std::endl;
    });

    // Start server
    if (!ipcServer.start()) {
        std::cerr << "Failed to start IPC server" << std::endl;
        return 1;
    }

    std::cout << "Arctic IPC Server running on ws://localhost:52010" << std::endl;

    // Main application loop
    while (g_running) {
        // Your render loop...

        // Broadcast events as needed
        if (g_sceneChanged) {
            ipcServer.broadcast("sceneUpdated", {
                {"changes", {"entities", "lights"}}
            });
            g_sceneChanged = false;
        }
    }

    // Cleanup
    ipcServer.stop();

    return 0;
}

*/
