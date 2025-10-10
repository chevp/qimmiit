package io.qimmiit.qilak.core;

/**
 * AgentModule - Base interface for Qilak modules
 *
 * Similar to IModule in Nanook (C++), but for server-side Java.
 * Modules contain business logic and orchestrate Akutik tools.
 */
public interface AgentModule {

    /**
     * Initialize the module
     *
     * @param agent Reference to the Qilak agent
     * @return true if initialization succeeded
     */
    boolean initialize(QilakAgent agent);

    /**
     * Shutdown the module
     */
    void shutdown();

    /**
     * Get the module name
     */
    String getName();

    /**
     * Get the module type
     */
    ModuleType getType();

    /**
     * Module types
     */
    enum ModuleType {
        AI_OPTIMIZER,      // AI-based optimization
        ASSET_ANALYZER,    // Asset analysis and recommendations
        PERFORMANCE_MONITOR, // Performance analysis
        COMPUTE_SERVICE,   // Background compute tasks
        STORAGE_SERVICE,   // Data persistence (uses Akutik tools)
        CUSTOM
    }
}
