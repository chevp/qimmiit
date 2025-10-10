package io.qimmiit.qilak.core;

import io.smallrye.config.ConfigMapping;

/**
 * Qilak Agent Configuration
 *
 * Loaded from application.yaml
 */
@ConfigMapping(prefix = "qilak")
public interface AgentConfig {

    /**
     * Agent name/identifier
     */
    String name();

    /**
     * gRPC server port
     */
    int grpcPort();

    /**
     * HTTP REST API port
     */
    int httpPort();

    /**
     * Storage configuration
     */
    StorageConfig storage();

    /**
     * AI model configuration
     */
    AiConfig ai();

    /**
     * Performance tuning
     */
    PerformanceConfig performance();

    interface StorageConfig {
        /**
         * Database path for persistent storage
         */
        String databasePath();

        /**
         * Cache size in MB
         */
        int cacheSizeMb();

        /**
         * Cache TTL in seconds
         */
        int cacheTtlSeconds();
    }

    interface AiConfig {
        /**
         * Enable AI-based optimization
         */
        boolean enabled();

        /**
         * Model path or endpoint
         */
        String modelPath();

        /**
         * Confidence threshold for recommendations
         */
        double confidenceThreshold();
    }

    interface PerformanceConfig {
        /**
         * Number of worker threads
         */
        int workerThreads();

        /**
         * Max concurrent requests
         */
        int maxConcurrentRequests();
    }
}
