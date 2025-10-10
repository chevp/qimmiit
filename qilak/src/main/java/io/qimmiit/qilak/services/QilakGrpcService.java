package io.qimmiit.qilak.services;

import io.grpc.stub.StreamObserver;
import io.qimmiit.qilak.core.DecisionRequest;
import io.qimmiit.qilak.core.DecisionResponse;
import io.qimmiit.qilak.core.QilakAgent;
import io.qimmiit.qilak.grpc.*;
import io.quarkus.grpc.GrpcService;
import io.smallrye.mutiny.Uni;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.util.HashMap;
import java.util.Map;

/**
 * Qilak gRPC Service Implementation
 *
 * Handles communication between Nanook (C++) and Qilak (Java)
 */
@GrpcService
public class QilakGrpcService implements QilakService {

    private static final Logger LOG = Logger.getLogger(QilakGrpcService.class);

    @Inject
    QilakAgent agent;

    private final long startTime = System.currentTimeMillis();

    @Override
    public Uni<io.qimmiit.qilak.grpc.DecisionResponse> makeDecision(
            io.qimmiit.qilak.grpc.DecisionRequest request) {

        LOG.infof("Received decision request: %s", request.getType());

        // Convert proto to internal model
        DecisionRequest internalRequest = toInternalRequest(request);

        // Process decision using agent
        return agent.processDecision(internalRequest)
                .map(this::toProtoResponse);
    }

    @Override
    public void makeDecisionStream(
            io.qimmiit.qilak.grpc.DecisionRequest request,
            StreamObserver<io.qimmiit.qilak.grpc.DecisionResponse> responseObserver) {

        LOG.infof("Received streaming decision request: %s", request.getType());

        // Convert proto to internal model
        DecisionRequest internalRequest = toInternalRequest(request);

        // Process decision
        agent.processDecision(internalRequest)
                .subscribe()
                .with(
                        response -> {
                            // Send response
                            responseObserver.onNext(toProtoResponse(response));
                            responseObserver.onCompleted();
                        },
                        error -> {
                            LOG.errorf("Error processing decision: %s", error.getMessage());
                            responseObserver.onError(error);
                        }
                );
    }

    @Override
    public Uni<HealthCheckResponse> healthCheck(HealthCheckRequest request) {
        LOG.debugf("Health check for service: %s", request.getService());

        boolean isRunning = agent.isRunning();

        return Uni.createFrom().item(
                HealthCheckResponse.newBuilder()
                        .setStatus(isRunning ?
                                HealthCheckResponse.Status.SERVING :
                                HealthCheckResponse.Status.NOT_SERVING)
                        .setMessage(isRunning ? "Qilak agent is running" : "Qilak agent is not running")
                        .build()
        );
    }

    @Override
    public Uni<StatusResponse> getStatus(StatusRequest request) {
        LOG.debug("Status request received");

        long uptimeSeconds = (System.currentTimeMillis() - startTime) / 1000;

        StatusResponse.Builder builder = StatusResponse.newBuilder()
                .setAgentName("Qilak")
                .setVersion("1.0.0")
                .setRunning(agent.isRunning())
                .setUptimeSeconds(uptimeSeconds);

        // TODO: Add module status if requested
        if (request.getIncludeModules()) {
            // builder.addModules(...)
        }

        return Uni.createFrom().item(builder.build());
    }

    /**
     * Convert proto DecisionRequest to internal model
     */
    private DecisionRequest toInternalRequest(io.qimmiit.qilak.grpc.DecisionRequest proto) {
        DecisionRequest.DecisionType type = switch (proto.getType()) {
            case OPTIMIZE_ASSET -> DecisionRequest.DecisionType.OPTIMIZE_ASSET;
            case ANALYZE_PERFORMANCE -> DecisionRequest.DecisionType.ANALYZE_PERFORMANCE;
            case RECOMMEND_LOD -> DecisionRequest.DecisionType.RECOMMEND_LOD;
            case GENERATE_SHADER_VARIANT -> DecisionRequest.DecisionType.GENERATE_SHADER_VARIANT;
            case COMPUTE_LIGHTING -> DecisionRequest.DecisionType.COMPUTE_LIGHTING;
            default -> DecisionRequest.DecisionType.UNKNOWN;
        };

        Map<String, Object> parameters = new HashMap<>(proto.getParametersMap());

        DecisionRequest request = new DecisionRequest(type, parameters);
        request.setRequestId(proto.getRequestId());
        request.setTimestamp(proto.getTimestamp());

        return request;
    }

    /**
     * Convert internal DecisionResponse to proto
     */
    private io.qimmiit.qilak.grpc.DecisionResponse toProtoResponse(DecisionResponse internal) {
        io.qimmiit.qilak.grpc.DecisionResponse.Builder builder =
                io.qimmiit.qilak.grpc.DecisionResponse.newBuilder()
                        .setStatus(internal.getStatus())
                        .setRecommendation(internal.getRecommendation())
                        .setConfidence(internal.getConfidence())
                        .setTimestamp(internal.getTimestamp());

        // Convert metadata
        if (internal.getMetadata() != null) {
            internal.getMetadata().forEach((key, value) ->
                    builder.putMetadata(key, value.toString())
            );
        }

        return builder.build();
    }
}
