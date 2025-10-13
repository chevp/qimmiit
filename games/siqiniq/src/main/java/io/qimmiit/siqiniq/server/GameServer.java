package io.qimmiit.siqiniq.server;

import io.quarkus.logging.Log;
import io.quarkus.scheduler.Scheduled;
import io.quarkus.runtime.StartupEvent;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.enterprise.event.Observes;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * Main game server - handles game loop and coordination
 */
@ApplicationScoped
public class GameServer {

    private final AtomicInteger tickCounter = new AtomicInteger(0);
    private volatile boolean running = false;

    void onStart(@Observes StartupEvent ev) {
        Log.info("=========================================");
        Log.info("Siqiniq-001 Game Server Starting...");
        Log.info("=========================================");

        // Initialize game systems
        initializeGameSystems();

        running = true;
        Log.info("Game server initialized successfully!");
    }

    private void initializeGameSystems() {
        Log.info("Initializing game systems...");
        // TODO: Initialize world manager, player manager, etc.
    }

    /**
     * Game tick - runs at 20 TPS (50ms intervals)
     */
    @Scheduled(every = "50ms")
    void gameTick() {
        if (!running) return;

        int tick = tickCounter.incrementAndGet();

        // Update game logic
        updateGameState();

        // Periodic logging
        if (tick % 1200 == 0) { // Every minute at 20 TPS
            Log.debug("Game tick: " + tick + " | Running: " + running);
        }
    }

    private void updateGameState() {
        // TODO: Update world, players, NPCs, combat, etc.
    }

    /**
     * Autosave - runs every 5 minutes
     */
    @Scheduled(every = "5m")
    void autosave() {
        Log.info("Autosaving game state...");
        // TODO: Save game state to database
    }

    public boolean isRunning() {
        return running;
    }

    public int getCurrentTick() {
        return tickCounter.get();
    }
}
