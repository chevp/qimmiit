/**
 * Qimmiit Agent Interface
 *
 * Base interface for all agents in the Qimmiit ecosystem.
 *
 * IMPORTANT: This interface is for agents (independent executables),
 * NOT for engine modules. Agents implementing this interface should
 * NOT depend on or link against any Qimmiit engine libraries.
 */

#pragma once

#include <string>
#include <map>
#include <cstdint>

namespace cryo {
namespace agents {

/**
 * Agent health status
 */
enum class HealthStatus {
    HEALTHY,
    DEGRADED,
    UNHEALTHY,
    UNKNOWN
};

/**
 * Agent metadata information
 */
struct AgentMetadata {
    std::string name;           // Agent name (e.g., "scene-optimizer")
    std::string version;        // Semantic version (e.g., "1.2.3")
    std::string description;    // Brief description
    std::string protocol;       // "grpc" or "http"
    int32_t port;              // Listening port
    std::string apiVersion;    // API version (e.g., "v1")
};

/**
 * Agent health information
 */
struct AgentHealth {
    HealthStatus status;
    std::string message;
    uint64_t uptime_seconds;
    std::map<std::string, std::string> details;  // Additional health info
};

/**
 * Agent metrics for monitoring
 */
struct AgentMetrics {
    uint64_t total_requests;
    uint64_t successful_requests;
    uint64_t failed_requests;
    uint64_t active_connections;
    double avg_response_time_ms;
    std::map<std::string, std::string> custom_metrics;
};

/**
 * Base interface for all agents
 *
 * Agents are independent executables that implement this interface
 * to provide standardized functionality.
 */
class IAgentModule {
public:
    virtual ~IAgentModule() = default;

    /**
     * Initialize the agent
     *
     * @param config Agent-specific configuration
     * @return true if initialization succeeded, false otherwise
     */
    virtual bool initialize(const std::map<std::string, std::string>& config) = 0;

    /**
     * Start the agent server
     *
     * This method should be non-blocking and start the agent's
     * gRPC or HTTP server in a background thread or event loop.
     *
     * @return true if server started successfully, false otherwise
     */
    virtual bool start() = 0;

    /**
     * Shutdown the agent gracefully
     *
     * This should stop accepting new requests and complete any
     * in-flight requests before shutting down.
     */
    virtual void shutdown() = 0;

    /**
     * Get agent metadata
     *
     * @return AgentMetadata structure with agent information
     */
    virtual AgentMetadata getMetadata() const = 0;

    /**
     * Check agent health
     *
     * Should be quick and non-blocking. Used by health check endpoints
     * and monitoring systems.
     *
     * @return AgentHealth structure with current health status
     */
    virtual AgentHealth checkHealth() const = 0;

    /**
     * Get current agent metrics
     *
     * @return AgentMetrics structure with performance metrics
     */
    virtual AgentMetrics getMetrics() const = 0;

    /**
     * Check if agent is running
     *
     * @return true if agent server is running, false otherwise
     */
    virtual bool isRunning() const = 0;
};

/**
 * Helper function to convert HealthStatus to string
 */
inline const char* healthStatusToString(HealthStatus status) {
    switch (status) {
        case HealthStatus::HEALTHY:   return "HEALTHY";
        case HealthStatus::DEGRADED:  return "DEGRADED";
        case HealthStatus::UNHEALTHY: return "UNHEALTHY";
        case HealthStatus::UNKNOWN:   return "UNKNOWN";
        default:                      return "INVALID";
    }
}

/**
 * Helper function to parse HealthStatus from string
 */
inline HealthStatus healthStatusFromString(const std::string& str) {
    if (str == "HEALTHY")   return HealthStatus::HEALTHY;
    if (str == "DEGRADED")  return HealthStatus::DEGRADED;
    if (str == "UNHEALTHY") return HealthStatus::UNHEALTHY;
    return HealthStatus::UNKNOWN;
}

} // namespace agents
} // namespace cryo
