package io.qimmiit.siqiniq.server.shader;

import io.grpc.stub.StreamObserver;
import io.quarkus.scheduler.Scheduled;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * AdaptiveLODSystem
 *
 * Automatic quality adaptation based on performance.
 *
 * LOD Levels:
 * 0 (Highest) - Full PBR with IBL, Normal mapping, Specular (~8-10ms)
 * 1 (High)    - PBR without IBL (~5-6ms)
 * 2 (Medium)  - Simplified with Specular only (~3-4ms)
 * 3 (Low)     - Basic Lambert diffuse (~1-2ms)
 *
 * Automatic adaptation:
 * - Frame time > 16.67ms → Drop LOD
 * - Frame time < 12ms → Increase LOD
 */
@ApplicationScoped
public class AdaptiveLODSystem {

    private static final Logger logger = LoggerFactory.getLogger(AdaptiveLODSystem.class);

    @Inject
    ShaderGraphManager shaderGraphManager;

    // Current LOD level (0-3)
    private volatile int currentLODLevel = 0;

    // Performance budget in milliseconds (default: 16.67ms = 60 FPS)
    private volatile float performanceBudgetMs = 16.67f;

    // Frame time tracking
    private volatile float currentFrameTimeMs = 0.0f;
    private final List<Float> frameTimeHistory = new ArrayList<>();
    private static final int FRAME_HISTORY_SIZE = 60;

    // Performance monitors
    private final Map<String, StreamObserver<Object /* PerformanceReport */>> performanceMonitors = new ConcurrentHashMap<>();

    // LOD change tracking
    private volatile long lastLODChangeTime = 0;
    private static final long LOD_CHANGE_COOLDOWN_MS = 2000; // 2 seconds

    /**
     * Scheduled task to monitor performance and adapt LOD
     * Runs every 100ms
     */
    @Scheduled(every = "100ms")
    void monitorPerformance() {
        // Update frame time history
        if (currentFrameTimeMs > 0) {
            frameTimeHistory.add(currentFrameTimeMs);
            if (frameTimeHistory.size() > FRAME_HISTORY_SIZE) {
                frameTimeHistory.remove(0);
            }
        }

        // Calculate average frame time
        float avgFrameTime = calculateAverageFrameTime();

        // Check if LOD change is needed
        if (shouldChangeLOD(avgFrameTime)) {
            adaptLOD(avgFrameTime);
        }

        // Send performance reports to monitors
        sendPerformanceReports(avgFrameTime);
    }

    /**
     * Set performance budget (target frame time)
     */
    public void setPerformanceBudget(float budgetMs) {
        logger.info("Setting performance budget: {}ms (was: {}ms)", budgetMs, this.performanceBudgetMs);
        this.performanceBudgetMs = budgetMs;
    }

    /**
     * Update current frame time (called by renderer)
     */
    public void updateFrameTime(float frameTimeMs) {
        this.currentFrameTimeMs = frameTimeMs;
    }

    /**
     * Manually set LOD level
     */
    public void setLODLevel(int lodLevel, StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        if (lodLevel < 0 || lodLevel > 3) {
            logger.error("Invalid LOD level: {}", lodLevel);
            return;
        }

        int oldLOD = currentLODLevel;
        currentLODLevel = lodLevel;
        lastLODChangeTime = System.currentTimeMillis();

        logger.info("LOD level changed: {} → {} (manual)", oldLOD, lodLevel);

        // Send LOD change response
        sendLODChangeResponse(oldLOD, lodLevel, responseObserver);
    }

    /**
     * Register performance monitor
     */
    public void registerPerformanceMonitor(String monitorId, StreamObserver<Object /* PerformanceReport */> observer) {
        performanceMonitors.put(monitorId, observer);
        logger.info("Registered performance monitor: {}", monitorId);
    }

    /**
     * Unregister performance monitor
     */
    public void unregisterPerformanceMonitor(String monitorId) {
        performanceMonitors.remove(monitorId);
        logger.info("Unregistered performance monitor: {}", monitorId);
    }

    // --- Private Methods ---

    private float calculateAverageFrameTime() {
        if (frameTimeHistory.isEmpty()) {
            return 0.0f;
        }
        float sum = 0.0f;
        for (float time : frameTimeHistory) {
            sum += time;
        }
        return sum / frameTimeHistory.size();
    }

    private boolean shouldChangeLOD(float avgFrameTime) {
        // Check cooldown
        long now = System.currentTimeMillis();
        if (now - lastLODChangeTime < LOD_CHANGE_COOLDOWN_MS) {
            return false;
        }

        // Check if frame time exceeds budget (drop LOD)
        if (avgFrameTime > performanceBudgetMs && currentLODLevel < 3) {
            return true;
        }

        // Check if frame time is well below budget (increase LOD)
        float headroom = performanceBudgetMs * 0.7f; // 70% of budget
        if (avgFrameTime < headroom && currentLODLevel > 0) {
            return true;
        }

        return false;
    }

    private void adaptLOD(float avgFrameTime) {
        int oldLOD = currentLODLevel;
        int newLOD = oldLOD;

        if (avgFrameTime > performanceBudgetMs) {
            // Frame time exceeded - drop LOD
            newLOD = Math.min(3, oldLOD + 1);
            logger.warn("Frame time {}ms > budget {}ms, dropping LOD: {} → {}",
                    avgFrameTime, performanceBudgetMs, oldLOD, newLOD);
        } else if (avgFrameTime < performanceBudgetMs * 0.7f) {
            // Frame time well below budget - increase LOD
            newLOD = Math.max(0, oldLOD - 1);
            logger.info("Frame time {}ms < budget {}ms, increasing LOD: {} → {}",
                    avgFrameTime, performanceBudgetMs, oldLOD, newLOD);
        }

        if (newLOD != oldLOD) {
            currentLODLevel = newLOD;
            lastLODChangeTime = System.currentTimeMillis();
            applyLODLevel(newLOD);
        }
    }

    private void applyLODLevel(int lodLevel) {
        logger.info("Applying LOD level: {}", lodLevel);

        // TODO: Enable/disable shader nodes based on LOD level
        switch (lodLevel) {
            case 0: // Full PBR
                enableNode("ibl_contribution");
                enableNode("normal_mapping");
                enableNode("specular_reflection");
                break;
            case 1: // PBR without IBL
                disableNode("ibl_contribution");
                enableNode("normal_mapping");
                enableNode("specular_reflection");
                break;
            case 2: // Simplified
                disableNode("ibl_contribution");
                disableNode("normal_mapping");
                enableNode("specular_reflection");
                break;
            case 3: // Basic
                disableNode("ibl_contribution");
                disableNode("normal_mapping");
                disableNode("specular_reflection");
                break;
        }
    }

    private void enableNode(String nodeId) {
        logger.debug("Enabling shader node: {}", nodeId);
        // TODO: Enable node in shader graph
    }

    private void disableNode(String nodeId) {
        logger.debug("Disabling shader node: {}", nodeId);
        // TODO: Disable node in shader graph
    }

    private void sendLODChangeResponse(int oldLOD, int newLOD,
                                        StreamObserver<Object /* ShaderGraphResponse */> responseObserver) {
        // TODO: Build LODChangedResponse
        // LODChangedResponse response = LODChangedResponse.newBuilder()
        //     .setOldLod(oldLOD)
        //     .setNewLod(newLOD)
        //     .addDisabledNodes(...)
        //     .addEnabledNodes(...)
        //     .setNewTotalCostMs(...)
        //     .build();
        // responseObserver.onNext(ShaderGraphResponse.newBuilder()
        //     .setLodChanged(response)
        //     .build());
    }

    private void sendPerformanceReports(float avgFrameTime) {
        if (performanceMonitors.isEmpty()) {
            return;
        }

        // Build performance report
        float budgetUtilization = (avgFrameTime / performanceBudgetMs) * 100.0f;
        boolean budgetExceeded = avgFrameTime > performanceBudgetMs;

        // TODO: Build PerformanceReport
        // PerformanceReport report = PerformanceReport.newBuilder()
        //     .setFrameTimeMs(avgFrameTime)
        //     .setTargetFrameTimeMs(performanceBudgetMs)
        //     .setBudgetUtilization(budgetUtilization)
        //     .setBudgetExceeded(budgetExceeded)
        //     .setCurrentLodLevel(currentLODLevel)
        //     .build();

        // Send to all monitors
        for (var entry : performanceMonitors.entrySet()) {
            try {
                // entry.getValue().onNext(report);
            } catch (Exception e) {
                logger.error("Error sending performance report to {}: {}", entry.getKey(), e.getMessage());
                unregisterPerformanceMonitor(entry.getKey());
            }
        }
    }

    // --- Metrics ---

    public int getCurrentLODLevel() {
        return currentLODLevel;
    }

    public float getPerformanceBudgetMs() {
        return performanceBudgetMs;
    }

    public float getCurrentFrameTimeMs() {
        return currentFrameTimeMs;
    }

    public float getAverageFrameTimeMs() {
        return calculateAverageFrameTime();
    }

    public int getActiveMonitorCount() {
        return performanceMonitors.size();
    }
}
