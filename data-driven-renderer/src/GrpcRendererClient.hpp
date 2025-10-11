#pragma once

#include "Config.hpp"

// Fix Windows macro conflicts with protobuf enums
#ifdef OPAQUE
#undef OPAQUE
#endif

#include <arctic_common.pb.h>
#include <arctic_authoring.pb.h>
#include <arctic_runtime.pb.h>
#include <arctic_network.pb.h>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>

namespace datadriven {

// Forward declare the shader manager
class ShaderInstanceManager;

/**
 * gRPC Client for RendererService
 *
 * Maintains bidirectional streaming connection to Quarkus backend.
 * Receives shader updates and sends rendering events.
 */
class GrpcRendererClient {
public:
    using ShaderUpdateCallback = std::function<void(const arctic::network::ShaderUpdate&)>;
    using SceneUpdateCallback = std::function<void(const arctic::network::SceneUpdate&)>;
    using UIStyleUpdateCallback = std::function<void(const arctic::network::UIStyleUpdate&)>;

    explicit GrpcRendererClient(const GrpcBackendConfig& config);
    ~GrpcRendererClient();

    /**
     * Connect to gRPC server and start streaming
     */
    bool connect();

    /**
     * Disconnect and cleanup
     */
    void disconnect();

    /**
     * Send frame rendered event to server
     */
    void sendFrameRendered(uint64_t frameNumber, float frameTimeMs);

    /**
     * Send user interaction event to server
     */
    void sendUserInteraction(const std::string& widgetId, const std::string& interactionType);

    /**
     * Set callback for shader updates from server
     */
    void setShaderUpdateCallback(ShaderUpdateCallback callback) {
        shaderUpdateCallback_ = callback;
    }

    /**
     * Set callback for scene updates from server
     */
    void setSceneUpdateCallback(SceneUpdateCallback callback) {
        sceneUpdateCallback_ = callback;
    }

    /**
     * Set callback for UI style updates from server
     */
    void setUIStyleUpdateCallback(UIStyleUpdateCallback callback) {
        uiStyleUpdateCallback_ = callback;
    }

    /**
     * Check if connected to server
     */
    bool isConnected() const { return connected_; }

private:
    void streamingThread();
    void processRenderingCommand(const arctic::network::RenderingCommand& command);

    GrpcBackendConfig config_;
    std::shared_ptr<grpc::Channel> channel_;

    ShaderUpdateCallback shaderUpdateCallback_;
    SceneUpdateCallback sceneUpdateCallback_;
    UIStyleUpdateCallback uiStyleUpdateCallback_;

    std::atomic<bool> connected_{false};
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> streamThread_;
};

} // namespace datadriven
