package io.qimmiit.siqiniq.server.render;

import io.grpc.stub.StreamObserver;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * RenderStreamManager
 *
 * Manages all active render stream connections and broadcasts events.
 *
 * Responsibilities:
 * - Track active connections
 * - Broadcast render events to all clients
 * - Manage connection lifecycle
 * - Provide connection metrics
 */
@ApplicationScoped
public class RenderStreamManager {

    private static final Logger logger = LoggerFactory.getLogger(RenderStreamManager.class);

    // Active connections: connectionId -> StreamObserver
    private final Map<String, StreamObserver<Object /* RenderEvent */>> activeConnections = new ConcurrentHashMap<>();

    // Metrics
    private final AtomicInteger totalConnectionsCounter = new AtomicInteger(0);
    private final AtomicInteger eventsSentCounter = new AtomicInteger(0);
    private final AtomicInteger eventsReceivedCounter = new AtomicInteger(0);

    /**
     * Register new render stream connection
     */
    public void registerConnection(String connectionId, StreamObserver<Object /* RenderEvent */> observer) {
        activeConnections.put(connectionId, observer);
        totalConnectionsCounter.incrementAndGet();
        logger.info("Registered connection: {} (active: {})", connectionId, activeConnections.size());

        // Send initial scene state
        sendInitialSceneState(connectionId, observer);
    }

    /**
     * Unregister render stream connection
     */
    public void unregisterConnection(String connectionId) {
        StreamObserver<?> removed = activeConnections.remove(connectionId);
        if (removed != null) {
            logger.info("Unregistered connection: {} (active: {})", connectionId, activeConnections.size());
        }
    }

    /**
     * Broadcast render event to all connected clients
     */
    public void broadcast(Object /* RenderEvent */ event) {
        int sent = 0;
        for (var entry : activeConnections.entrySet()) {
            try {
                entry.getValue().onNext(event);
                sent++;
            } catch (Exception e) {
                logger.error("Error sending to {}: {}", entry.getKey(), e.getMessage());
                // Remove failed connection
                unregisterConnection(entry.getKey());
            }
        }
        eventsSentCounter.addAndGet(sent);
    }

    /**
     * Send render event to specific connection
     */
    public void sendToConnection(String connectionId, Object /* RenderEvent */ event) {
        StreamObserver<Object> observer = activeConnections.get(connectionId);
        if (observer != null) {
            try {
                observer.onNext(event);
                eventsSentCounter.incrementAndGet();
            } catch (Exception e) {
                logger.error("Error sending to {}: {}", connectionId, e.getMessage());
                unregisterConnection(connectionId);
            }
        }
    }

    /**
     * Send initial scene state to newly connected client
     */
    private void sendInitialSceneState(String connectionId, StreamObserver<Object /* RenderEvent */> observer) {
        try {
            // TODO: Build and send SceneLoaded event
            // SceneLoaded sceneLoaded = SceneLoaded.newBuilder()
            //     .setSceneId("default")
            //     .build();
            // RenderEvent event = RenderEvent.newBuilder()
            //     .setSceneLoaded(sceneLoaded)
            //     .build();
            // observer.onNext(event);

            logger.debug("Sent initial scene state to {}", connectionId);
        } catch (Exception e) {
            logger.error("Error sending initial state to {}: {}", connectionId, e.getMessage());
        }
    }

    /**
     * Build scene snapshot for GetSceneSnapshot RPC
     */
    public Object /* SceneSnapshot */ buildSceneSnapshot(String sceneId) {
        // TODO: Build scene snapshot from current state
        // SceneSnapshot.Builder builder = SceneSnapshot.newBuilder()
        //     .setSceneId(sceneId)
        //     .setTimestamp(System.currentTimeMillis());

        // Add all entities, camera, lights, materials

        // return builder.build();
        return null;
    }

    // --- Metrics ---

    public int getActiveConnectionCount() {
        return activeConnections.size();
    }

    public int getTotalConnectionCount() {
        return totalConnectionsCounter.get();
    }

    public int getEventsSentCount() {
        return eventsSentCounter.get();
    }

    public int getEventsReceivedCount() {
        return eventsReceivedCounter.get();
    }

    public void incrementEventsReceived() {
        eventsReceivedCounter.incrementAndGet();
    }

    // --- Event Broadcasting Methods ---

    /**
     * Broadcast entity spawned event
     */
    public void broadcastEntitySpawned(String entityId, Object /* Transform */ transform) {
        // TODO: Build and broadcast EntitySpawned event
        logger.debug("Broadcasting entity spawned: {}", entityId);
    }

    /**
     * Broadcast entity updated event
     */
    public void broadcastEntityUpdated(String entityId, Object /* Transform */ transform) {
        // TODO: Build and broadcast EntityUpdated event
        logger.debug("Broadcasting entity updated: {}", entityId);
    }

    /**
     * Broadcast entity destroyed event
     */
    public void broadcastEntityDestroyed(String entityId) {
        // TODO: Build and broadcast EntityDestroyed event
        logger.debug("Broadcasting entity destroyed: {}", entityId);
    }

    /**
     * Broadcast camera update event
     */
    public void broadcastCameraUpdate(Object /* CameraState */ cameraState) {
        // TODO: Build and broadcast CameraUpdate event
        logger.debug("Broadcasting camera update");
    }

    /**
     * Broadcast frame stats event
     */
    public void broadcastFrameStats(float fps, int drawCalls, float frameTimeMs) {
        // TODO: Build and broadcast FrameStats event
        logger.debug("Broadcasting frame stats: FPS={}, DrawCalls={}, FrameTime={}ms", fps, drawCalls, frameTimeMs);
    }
}
