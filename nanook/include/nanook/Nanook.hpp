/**
 * @file Nanook.hpp
 * @brief Simple Nanook wrapper for Nunaq launcher
 *
 * Nanook - Module orchestrator for the Qimmiit engine.
 * This is a simple interface that wraps CentralAgent for easy use from Nunaq.
 */

#pragma once

// Forward declare GLFW types
struct GLFWwindow;

namespace nanook {

/**
 * @brief Simple Nanook orchestrator wrapper
 *
 * This class provides a simple interface for Nunaq to initialize
 * and run the Nanook engine orchestrator.
 */
class Nanook {
public:
    /**
     * @brief Constructor
     */
    Nanook();

    /**
     * @brief Destructor
     */
    ~Nanook();

    /**
     * @brief Initialize Nanook with window and arguments
     * @param window GLFW window created by Nunaq
     * @param argc Command line argument count
     * @param argv Command line arguments
     * @return true if initialization succeeded
     */
    bool initialize(GLFWwindow* window, int argc, char* argv[]);

    /**
     * @brief Run the main engine loop
     *
     * This will run until the window is closed or shutdown is requested.
     */
    void run();

    /**
     * @brief Shutdown Nanook
     */
    void shutdown();

    /**
     * @brief Check if Nanook is running
     * @return true if running
     */
    bool isRunning() const;

private:
    class Impl;
    Impl* impl_;  // PIMPL to hide CentralAgent details
};

} // namespace nanook
