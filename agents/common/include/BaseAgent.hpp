/**
 * Base Agent Implementation
 *
 * Provides a default implementation of IAgentModule that handles
 * common functionality like metrics tracking, health checks, and
 * lifecycle management.
 *
 * Agent implementations should inherit from this class and implement
 * the agent-specific functionality.
 */

#pragma once

#include "IAgentModule.hpp"
#include <atomic>
#include <chrono>
#include <mutex>

namespace cryo {
namespace agents {

/**
 * Base agent implementation with common functionality
 */
class BaseAgent : public IAgentModule {
public:
    BaseAgent(const std::string& name, const std::string& version, const std::string& protocol)
        : name_(name)
        , version_(version)
        , protocol_(protocol)
        , running_(false)
        , startTime_(0)
        , totalRequests_(0)
        , successfulRequests_(0)
        , failedRequests_(0)
        , activeConnections_(0)
        , totalResponseTime_(0.0)
    {
    }

    virtual ~BaseAgent() = default;

    // IAgentModule implementation

    bool initialize(const std::map<std::string, std::string>& config) override {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;

        // Store port from config
        auto portIt = config.find("port");
        if (portIt != config.end()) {
            port_ = std::stoi(portIt->second);
        }

        // Store description from config
        auto descIt = config.find("description");
        if (descIt != config.end()) {
            description_ = descIt->second;
        }

        // Store API version from config
        auto apiVersionIt = config.find("api_version");
        if (apiVersionIt != config.end()) {
            apiVersion_ = apiVersionIt->second;
        }

        return onInitialize(config);
    }

    bool start() override {
        if (running_) {
            return false; // Already running
        }

        startTime_ = std::chrono::system_clock::now().time_since_epoch().count() / 1000000000;
        running_ = true;

        return onStart();
    }

    void shutdown() override {
        if (!running_) {
            return; // Not running
        }

        running_ = false;
        onShutdown();
    }

    AgentMetadata getMetadata() const override {
        AgentMetadata metadata;
        metadata.name = name_;
        metadata.version = version_;
        metadata.description = description_;
        metadata.protocol = protocol_;
        metadata.port = port_;
        metadata.apiVersion = apiVersion_;
        return metadata;
    }

    AgentHealth checkHealth() const override {
        AgentHealth health;
        health.status = running_ ? HealthStatus::HEALTHY : HealthStatus::UNHEALTHY;
        health.message = running_ ? "Agent is running" : "Agent is not running";
        health.uptime_seconds = getUptimeSeconds();

        // Add custom health details from derived class
        onCheckHealth(health);

        return health;
    }

    AgentMetrics getMetrics() const override {
        std::lock_guard<std::mutex> lock(mutex_);

        AgentMetrics metrics;
        metrics.total_requests = totalRequests_;
        metrics.successful_requests = successfulRequests_;
        metrics.failed_requests = failedRequests_;
        metrics.active_connections = activeConnections_;

        if (totalRequests_ > 0) {
            metrics.avg_response_time_ms = totalResponseTime_ / totalRequests_;
        } else {
            metrics.avg_response_time_ms = 0.0;
        }

        // Add custom metrics from derived class
        onGetMetrics(metrics);

        return metrics;
    }

    bool isRunning() const override {
        return running_;
    }

protected:
    /**
     * Agent-specific initialization
     * Override this to add custom initialization logic
     *
     * @param config Configuration map
     * @return true if successful, false otherwise
     */
    virtual bool onInitialize(const std::map<std::string, std::string>& config) {
        return true;
    }

    /**
     * Agent-specific startup logic
     * Override this to start your gRPC/HTTP server
     *
     * @return true if successful, false otherwise
     */
    virtual bool onStart() = 0;

    /**
     * Agent-specific shutdown logic
     * Override this to gracefully stop your server
     */
    virtual void onShutdown() = 0;

    /**
     * Add custom health check details
     * Override this to add agent-specific health information
     *
     * @param health AgentHealth structure to populate
     */
    virtual void onCheckHealth(AgentHealth& health) const {
        // Default implementation does nothing
    }

    /**
     * Add custom metrics
     * Override this to add agent-specific metrics
     *
     * @param metrics AgentMetrics structure to populate
     */
    virtual void onGetMetrics(AgentMetrics& metrics) const {
        // Default implementation does nothing
    }

    // Helper methods for derived classes

    /**
     * Record a request
     * Call this when a request is received
     */
    void recordRequest() {
        std::lock_guard<std::mutex> lock(mutex_);
        totalRequests_++;
        activeConnections_++;
    }

    /**
     * Record a successful request
     * Call this when a request completes successfully
     *
     * @param response_time_ms Response time in milliseconds
     */
    void recordSuccess(double response_time_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        successfulRequests_++;
        activeConnections_--;
        totalResponseTime_ += response_time_ms;
    }

    /**
     * Record a failed request
     * Call this when a request fails
     *
     * @param response_time_ms Response time in milliseconds
     */
    void recordFailure(double response_time_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        failedRequests_++;
        activeConnections_--;
        totalResponseTime_ += response_time_ms;
    }

    /**
     * Get configuration value
     *
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return Configuration value or default
     */
    std::string getConfigValue(const std::string& key, const std::string& defaultValue = "") const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = config_.find(key);
        return it != config_.end() ? it->second : defaultValue;
    }

    /**
     * Get uptime in seconds
     *
     * @return Uptime in seconds
     */
    uint64_t getUptimeSeconds() const {
        if (!running_ || startTime_ == 0) {
            return 0;
        }
        uint64_t now = std::chrono::system_clock::now().time_since_epoch().count() / 1000000000;
        return now - startTime_;
    }

private:
    // Agent metadata
    std::string name_;
    std::string version_;
    std::string description_;
    std::string protocol_;
    int32_t port_ = 0;
    std::string apiVersion_ = "v1";

    // Runtime state
    std::atomic<bool> running_;
    uint64_t startTime_;

    // Configuration
    std::map<std::string, std::string> config_;

    // Metrics
    mutable std::mutex mutex_;
    uint64_t totalRequests_;
    uint64_t successfulRequests_;
    uint64_t failedRequests_;
    uint64_t activeConnections_;
    double totalResponseTime_;
};

} // namespace agents
} // namespace cryo
