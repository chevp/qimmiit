package io.qimmiit.siqiniq.server.shader;

import io.grpc.stub.StreamObserver;
import io.quarkus.grpc.GrpcService;
import jakarta.inject.Inject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

// Import from arctic-formats (will be generated)
// import io.qimmiit.arctic.shader.*;

/**
 * ShaderGraphStreamingService Implementation
 *
 * From: arctic_shader_streaming.proto
 * Port: 9001 (separate port)
 * Purpose: Advanced shader pipeline management with adaptive LOD
 *
 * Service Definition:
 * service ShaderGraphStreamingService {
 *   rpc StreamShaderGraphs(stream ShaderGraphCommand) returns (stream ShaderGraphResponse);
 *   rpc LoadGraph(LoadShaderGraphCmd) returns (ShaderGraphResponse);
 *   rpc MonitorPerformance(arctic.common.RpcVoid) returns (stream PerformanceReport);
 * }
 */
@GrpcService
public class ShaderGraphStreamingServiceImpl /* implements ShaderGraphStreamingService */ {

    private static final Logger logger = LoggerFactory.getLogger(ShaderGraphStreamingServiceImpl.class);

    @Inject
    ShaderGraphManager shaderGraphManager;

    @Inject
    AdaptiveLODSystem lodSystem;

    /**
     * Bidirectional streaming for real-time shader graph updates
     *
     * Commands (Client → Server):
     * - LoadShaderGraphCmd - Load complete shader graph
     * - UpdateNodeCmd - Hot-reload single shader node
     * - SetPerformanceBudgetCmd - Set performance budget (triggers adaptive LOD)
     * - SetLODLevelCmd - Manually set LOD level (0-3)
     * - ReplaceNodeCmd - Swap expensive node with cheaper alternative
     * - RecompileGraphCmd - Recompile entire graph after changes
     * - SwapShaderCmd - Swap entire shader (e.g., PBR → Unlit)
     *
     * Responses (Server → Client):
     * - GraphLoadedResponse - Graph loaded with cost estimate
     * - NodeUpdatedResponse - Node updated, recompilation status
     * - LODChangedResponse - LOD changed, nodes disabled/enabled
     * - PerformanceReport - Budget utilization, recommendations
     * - ShaderCompilationError - Compilation error with line/column
     */
    // @Override
    public StreamObserver<Object /* ShaderGraphCommand */> streamShaderGraphs(
            StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {

        String sessionId = java.util.UUID.randomUUID().toString();
        logger.info("New shader graph streaming session: {}", sessionId);

        return new StreamObserver<Object /* ShaderGraphCommand */>() {
            @Override
            public void onNext(Object /* ShaderGraphCommand */ command) {
                try {
                    // Process command based on type
                    // if (command.hasLoadGraph()) {
                    //     handleLoadGraph(sessionId, command.getLoadGraph(), responseObserver);
                    // } else if (command.hasUpdateNode()) {
                    //     handleUpdateNode(sessionId, command.getUpdateNode(), responseObserver);
                    // } else if (command.hasSetPerformanceBudget()) {
                    //     handleSetPerformanceBudget(sessionId, command.getSetPerformanceBudget(), responseObserver);
                    // } else if (command.hasSetLodLevel()) {
                    //     handleSetLODLevel(sessionId, command.getSetLodLevel(), responseObserver);
                    // } else if (command.hasReplaceNode()) {
                    //     handleReplaceNode(sessionId, command.getReplaceNode(), responseObserver);
                    // } else if (command.hasRecompileGraph()) {
                    //     handleRecompileGraph(sessionId, command.getRecompileGraph(), responseObserver);
                    // } else if (command.hasSwapShader()) {
                    //     handleSwapShader(sessionId, command.getSwapShader(), responseObserver);
                    // }

                    logger.debug("Processed shader graph command for session {}", sessionId);
                } catch (Exception e) {
                    logger.error("Error processing command for {}: {}", sessionId, e.getMessage(), e);
                    // Send error response
                    sendCompilationError(responseObserver, e.getMessage());
                }
            }

            @Override
            public void onError(Throwable t) {
                logger.error("Stream error for {}: {}", sessionId, t.getMessage(), t);
            }

            @Override
            public void onCompleted() {
                logger.info("Shader graph streaming completed for {}", sessionId);
                responseObserver.onCompleted();
            }
        };
    }

    /**
     * Unary RPC for one-time graph loads
     */
    // @Override
    public void loadGraph(
            Object /* LoadShaderGraphCmd */ request,
            StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {

        try {
            String graphId = "pbr_dynamic"; // request.getGraph().getGraphId();
            logger.info("Loading shader graph: {}", graphId);

            // Load graph
            // ShaderGraph graph = shaderGraphManager.loadGraph(request.getGraph());
            // float costMs = shaderGraphManager.estimateCost(graph);

            // Build response
            // GraphLoadedResponse response = GraphLoadedResponse.newBuilder()
            //     .setGraphId(graphId)
            //     .setEstimatedCostMs(costMs)
            //     .setSuccess(true)
            //     .build();

            // responseObserver.onNext(ShaderGraphResponse.newBuilder()
            //     .setGraphLoaded(response)
            //     .build());
            responseObserver.onCompleted();

            logger.info("Shader graph loaded: {} (cost: {}ms)", graphId, 0.0f);

        } catch (Exception e) {
            logger.error("Error loading graph: {}", e.getMessage(), e);
            sendCompilationError(responseObserver, e.getMessage());
            responseObserver.onCompleted();
        }
    }

    /**
     * Server streaming for performance telemetry
     *
     * Continuously sends performance reports including:
     * - Current FPS and frame time
     * - Budget utilization
     * - Automatic LOD changes
     * - Cost per shader node
     * - Recommendations for optimization
     */
    // @Override
    public void monitorPerformance(
            Object /* RpcVoid */ request,
            StreamObserver<Object /* PerformanceReport */> responseObserver) {

        String monitorId = java.util.UUID.randomUUID().toString();
        logger.info("Starting performance monitoring: {}", monitorId);

        // Register monitor
        lodSystem.registerPerformanceMonitor(monitorId, responseObserver);

        // Monitor will continuously send reports until client disconnects
        // Reports are sent by AdaptiveLODSystem based on frame timing
    }

    // --- Command Handlers ---

    private void handleLoadGraph(String sessionId, Object /* LoadShaderGraphCmd */ cmd,
                                  StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Load complete shader graph
        logger.info("Loading shader graph for session {}", sessionId);
    }

    private void handleUpdateNode(String sessionId, Object /* UpdateNodeCmd */ cmd,
                                   StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Hot-reload single shader node
        logger.info("Updating shader node for session {}", sessionId);

        // String nodeId = cmd.getNodeId();
        // shaderGraphManager.updateNode(nodeId, cmd.getUpdatedNode());
        // boolean recompileRequired = cmd.getRelinkConnections();
        // float newCost = shaderGraphManager.estimateNodeCost(nodeId);

        // Send response
        // NodeUpdatedResponse response = NodeUpdatedResponse.newBuilder()
        //     .setNodeId(nodeId)
        //     .setRecompilationRequired(recompileRequired)
        //     .setNewCostMs(newCost)
        //     .build();
        // responseObserver.onNext(ShaderGraphResponse.newBuilder()
        //     .setNodeUpdated(response)
        //     .build());
    }

    private void handleSetPerformanceBudget(String sessionId, Object /* SetPerformanceBudgetCmd */ cmd,
                                             StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Set performance budget - triggers adaptive LOD
        float budgetMs = 16.67f; // cmd.getTargetFrameTimeMs();
        logger.info("Setting performance budget: {}ms for session {}", budgetMs, sessionId);

        lodSystem.setPerformanceBudget(budgetMs);
    }

    private void handleSetLODLevel(String sessionId, Object /* SetLODLevelCmd */ cmd,
                                    StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Manually set LOD level
        int lodLevel = 0; // cmd.getLodLevel();
        logger.info("Setting LOD level: {} for session {}", lodLevel, sessionId);

        lodSystem.setLODLevel(lodLevel, responseObserver);
    }

    private void handleReplaceNode(String sessionId, Object /* ReplaceNodeCmd */ cmd,
                                    StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Swap expensive node with cheaper alternative
        logger.info("Replacing shader node for session {}", sessionId);
    }

    private void handleRecompileGraph(String sessionId, Object /* RecompileGraphCmd */ cmd,
                                       StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Recompile entire graph after changes
        logger.info("Recompiling shader graph for session {}", sessionId);
    }

    private void handleSwapShader(String sessionId, Object /* SwapShaderCmd */ cmd,
                                   StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // Swap entire shader (e.g., PBR → Unlit)
        logger.info("Swapping shader for session {}", sessionId);
    }

    private void sendCompilationError(StreamObserver<Object /* ShaderGraphResponse */> responseObserver,
                                       String errorMessage) {
        // TODO: Build ShaderCompilationError response
        // ShaderCompilationError error = ShaderCompilationError.newBuilder()
        //     .setErrorMessage(errorMessage)
        //     .build();
        // responseObserver.onNext(ShaderGraphResponse.newBuilder()
        //     .setCompilationError(error)
        //     .build());
    }
}
