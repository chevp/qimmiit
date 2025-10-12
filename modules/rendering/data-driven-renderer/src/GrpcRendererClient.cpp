#include "GrpcRendererClient.hpp"
#include <coregfx/core/ocean_log.hpp>

namespace datadriven {

GrpcRendererClient::GrpcRendererClient(const GrpcBackendConfig& config)
    : config_(config) {
    ocean::info("GrpcRendererClient created for: " + config_.address);
}

GrpcRendererClient::~GrpcRendererClient() {
    disconnect();
}

bool GrpcRendererClient::connect() {
    ocean::info("Connecting to gRPC server: " + config_.address);

    // Create channel
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(-1);  // Unlimited message size

    if (config_.tls) {
        auto creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
        channel_ = grpc::CreateCustomChannel(config_.address, creds, args);
    } else {
        channel_ = grpc::CreateCustomChannel(config_.address,
                                             grpc::InsecureChannelCredentials(),
                                             args);
    }

    if (!channel_) {
        ocean::error("Failed to create gRPC channel");
        return false;
    }

    // Check if server is available (with timeout)
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(1);
    if (!channel_->WaitForConnected(deadline)) {
        ocean::warn("gRPC server not available at: " + config_.address + " (continuing anyway)");
        // Don't fail completely - allow running without server for testing
    } else {
        ocean::info("gRPC channel connected successfully");
    }

    connected_ = true;
    running_ = true;

    // Start streaming thread (commented out until we have full gRPC service implementation)
    // streamThread_ = std::make_unique<std::thread>(&GrpcRendererClient::streamingThread, this);

    ocean::info("gRPC client started (streaming disabled - TODO: implement service stub)");
    return true;
}

void GrpcRendererClient::disconnect() {
    if (!connected_) {
        return;
    }

    ocean::info("Disconnecting from gRPC server...");

    running_ = false;

    if (streamThread_ && streamThread_->joinable()) {
        streamThread_->join();
    }

    connected_ = false;
    channel_.reset();

    ocean::info("gRPC client disconnected");
}

void GrpcRendererClient::sendFrameRendered(uint64_t frameNumber, float frameTimeMs) {
    if (!connected_) {
        return;
    }

    // TODO: Implement when gRPC service stub is available
    // arctic::RenderingEvent event;
    // auto* frameRendered = event.mutable_frame_rendered();
    // frameRendered->set_frame_number(frameNumber);
    // frameRendered->set_frame_time_ms(frameTimeMs);
    // writer_->Write(event);
}

void GrpcRendererClient::sendUserInteraction(const std::string& widgetId, const std::string& interactionType) {
    if (!connected_) {
        return;
    }

    ocean::info("User interaction: " + widgetId + " (" + interactionType + ")");

    // TODO: Implement when gRPC service stub is available
    // arctic::RenderingEvent event;
    // auto* interaction = event.mutable_user_interaction();
    // interaction->set_widget_id(widgetId);
    // interaction->set_interaction_type(interactionType);
    // writer_->Write(event);
}

void GrpcRendererClient::streamingThread() {
    ocean::info("gRPC streaming thread started");

    // TODO: Implement bidirectional streaming when service stub is available
    // This would create the stub, initiate the stream, and process incoming commands

    /*
    auto stub = arctic::RendererService::NewStub(channel_);
    grpc::ClientContext context;

    auto stream = stub->StreamRenderingUpdates(&context);

    while (running_) {
        arctic::network::RenderingCommand command;
        if (stream->Read(&command)) {
            processRenderingCommand(command);
        } else {
            // Stream ended or error
            break;
        }
    }

    stream->WritesDone();
    auto status = stream->Finish();
    */

    ocean::info("gRPC streaming thread ended");
}

void GrpcRendererClient::processRenderingCommand(const arctic::network::RenderingCommand& command) {
    switch (command.command_type_case()) {
        case arctic::network::RenderingCommand::kShaderUpdate:
            if (shaderUpdateCallback_) {
                shaderUpdateCallback_(command.shader_update());
            }
            break;

        case arctic::network::RenderingCommand::kSceneUpdate:
            if (sceneUpdateCallback_) {
                sceneUpdateCallback_(command.scene_update());
            }
            break;

        case arctic::network::RenderingCommand::kUiStyleUpdate:
            if (uiStyleUpdateCallback_) {
                uiStyleUpdateCallback_(command.ui_style_update());
            }
            break;

        default:
            ocean::warn("Unknown rendering command type");
            break;
    }
}

} // namespace datadriven
