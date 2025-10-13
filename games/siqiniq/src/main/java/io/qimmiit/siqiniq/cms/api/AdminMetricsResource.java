package io.qimmiit.siqiniq.cms.api;

import io.qimmiit.siqiniq.server.render.RenderStreamManager;
import io.qimmiit.siqiniq.server.shader.AdaptiveLODSystem;
import io.qimmiit.siqiniq.server.shader.ShaderGraphManager;
import jakarta.inject.Inject;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;

import java.lang.management.ManagementFactory;
import java.lang.management.MemoryMXBean;
import java.lang.management.OperatingSystemMXBean;
import java.util.Map;

/**
 * Admin API for server metrics and monitoring
 *
 * This is the ONLY REST API for siqiniq-001.
 * Provides admin/monitoring endpoints for:
 * - Active gRPC connections (RenderStreamService)
 * - Performance metrics (CPU, memory, frame time)
 * - Frame statistics (FPS, LOD level, shader cost)
 * - Resource usage
 *
 * Note: Business logic (worlds, items, characters) is NOT provided.
 * This is a Render Stream Server, not a traditional game server.
 */
@Path("/api/admin/metrics")
@Produces(MediaType.APPLICATION_JSON)
@Tag(name = "Admin Metrics", description = "Server monitoring and metrics")
public class AdminMetricsResource {

    @Inject
    RenderStreamManager renderStreamManager;

    @Inject
    AdaptiveLODSystem adaptiveLODSystem;

    @Inject
    ShaderGraphManager shaderGraphManager;

    @GET
    @Path("/summary")
    @Operation(summary = "Get server metrics summary")
    public Response getMetricsSummary() {
        OperatingSystemMXBean osBean = ManagementFactory.getOperatingSystemMXBean();
        MemoryMXBean memoryBean = ManagementFactory.getMemoryMXBean();

        long usedMemory = memoryBean.getHeapMemoryUsage().getUsed();
        long maxMemory = memoryBean.getHeapMemoryUsage().getMax();

        return Response.ok(Map.of(
            "server", Map.of(
                "type", "Render Stream Server",
                "uptime_ms", ManagementFactory.getRuntimeMXBean().getUptime(),
                "grpc_port", 9000,
                "rest_port", 8080
            ),
            "performance", Map.of(
                "cpu_load", osBean.getSystemLoadAverage(),
                "memory_used_mb", usedMemory / (1024 * 1024),
                "memory_max_mb", maxMemory / (1024 * 1024),
                "memory_percent", (usedMemory * 100.0) / maxMemory,
                "frame_time_ms", adaptiveLODSystem.getAverageFrameTimeMs(),
                "current_lod_level", adaptiveLODSystem.getCurrentLODLevel()
            ),
            "connections", Map.of(
                "active_streams", renderStreamManager.getActiveConnectionCount(),
                "total_connections", renderStreamManager.getTotalConnectionCount(),
                "events_sent", renderStreamManager.getEventsSentCount(),
                "events_received", renderStreamManager.getEventsReceivedCount()
            ),
            "shader_graphs", Map.of(
                "loaded_graphs", shaderGraphManager.getLoadedGraphCount(),
                "performance_monitors", adaptiveLODSystem.getActiveMonitorCount()
            )
        )).build();
    }

    @GET
    @Path("/connections")
    @Operation(summary = "Get detailed connection information for RenderStreamService")
    public Response getConnections() {
        return Response.ok(Map.of(
            "render_streams", Map.of(
                "active", renderStreamManager.getActiveConnectionCount(),
                "total_established", renderStreamManager.getTotalConnectionCount(),
                "events_sent", renderStreamManager.getEventsSentCount(),
                "events_received", renderStreamManager.getEventsReceivedCount()
            ),
            "shader_streams", Map.of(
                "active_monitors", adaptiveLODSystem.getActiveMonitorCount(),
                "loaded_graphs", shaderGraphManager.getLoadedGraphCount()
            )
            // TODO: Add per-connection details (connection ID, IP, uptime, event count)
        )).build();
    }

    @GET
    @Path("/performance")
    @Operation(summary = "Get performance metrics including LOD system")
    public Response getPerformanceMetrics() {
        OperatingSystemMXBean osBean = ManagementFactory.getOperatingSystemMXBean();
        MemoryMXBean memoryBean = ManagementFactory.getMemoryMXBean();

        return Response.ok(Map.of(
            "cpu", Map.of(
                "system_load", osBean.getSystemLoadAverage(),
                "available_processors", osBean.getAvailableProcessors()
            ),
            "memory", Map.of(
                "heap_used_mb", memoryBean.getHeapMemoryUsage().getUsed() / (1024 * 1024),
                "heap_max_mb", memoryBean.getHeapMemoryUsage().getMax() / (1024 * 1024),
                "heap_committed_mb", memoryBean.getHeapMemoryUsage().getCommitted() / (1024 * 1024),
                "non_heap_used_mb", memoryBean.getNonHeapMemoryUsage().getUsed() / (1024 * 1024)
            ),
            "rendering", Map.of(
                "frame_time_ms", adaptiveLODSystem.getCurrentFrameTimeMs(),
                "avg_frame_time_ms", adaptiveLODSystem.getAverageFrameTimeMs(),
                "performance_budget_ms", adaptiveLODSystem.getPerformanceBudgetMs(),
                "current_lod_level", adaptiveLODSystem.getCurrentLODLevel(),
                "target_fps", 60.0,
                "actual_fps", adaptiveLODSystem.getAverageFrameTimeMs() > 0 ?
                    1000.0 / adaptiveLODSystem.getAverageFrameTimeMs() : 0.0
            )
        )).build();
    }

    @GET
    @Path("/resources")
    @Operation(summary = "Get shader graph resource metrics")
    public Response getResourceMetrics() {
        return Response.ok(Map.of(
            "shader_graphs", Map.of(
                "loaded", shaderGraphManager.getLoadedGraphCount(),
                "graph_ids", shaderGraphManager.getLoadedGraphIds()
            ),
            "shader_nodes", Map.of(
                "node_costs", shaderGraphManager.getNodeCosts(),
                "high_cost_nodes", shaderGraphManager.getNodeCosts().entrySet().stream()
                    .filter(e -> e.getValue() > 2.0f)
                    .collect(java.util.stream.Collectors.toMap(
                        java.util.Map.Entry::getKey,
                        java.util.Map.Entry::getValue
                    ))
            ),
            "connections", Map.of(
                "render_streams", renderStreamManager.getActiveConnectionCount(),
                "performance_monitors", adaptiveLODSystem.getActiveMonitorCount()
            )
            // TODO: Add entity counts, scene data when implemented
        )).build();
    }

    @GET
    @Path("/frame-stats")
    @Operation(summary = "Get frame rendering statistics and LOD information")
    public Response getFrameStats() {
        float avgFrameTime = adaptiveLODSystem.getAverageFrameTimeMs();
        float currentFPS = avgFrameTime > 0 ? 1000.0f / avgFrameTime : 0.0f;

        return Response.ok(Map.of(
            "timing", Map.of(
                "current_frame_time_ms", adaptiveLODSystem.getCurrentFrameTimeMs(),
                "average_frame_time_ms", avgFrameTime,
                "target_frame_time_ms", adaptiveLODSystem.getPerformanceBudgetMs(),
                "fps", currentFPS,
                "target_fps", 1000.0 / adaptiveLODSystem.getPerformanceBudgetMs()
            ),
            "lod", Map.of(
                "current_level", adaptiveLODSystem.getCurrentLODLevel(),
                "auto_adapt_enabled", true,  // TODO: Make configurable
                "level_0_cost_ms", "8-10 (Full PBR + IBL)",
                "level_1_cost_ms", "5-6 (PBR - IBL)",
                "level_2_cost_ms", "3-4 (Simplified)",
                "level_3_cost_ms", "1-2 (Basic)"
            ),
            "connections", Map.of(
                "active_render_streams", renderStreamManager.getActiveConnectionCount(),
                "events_sent", renderStreamManager.getEventsSentCount()
            )
            // TODO: Add actual draw calls, vertices, triangles when renderer is connected
        )).build();
    }

    @POST
    @Path("/reset")
    @Operation(summary = "Reset metrics counters")
    public Response resetMetrics() {
        // TODO: Implement metrics reset
        return Response.ok(Map.of(
            "message", "Metrics reset requested",
            "timestamp", System.currentTimeMillis()
        )).build();
    }
}
