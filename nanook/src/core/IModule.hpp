/**
 * Nanook Engine - Module Interface
 */

#pragma once

#include "Types.hpp"
#include "ModuleSpec.hpp"
#include <string>
#include <vector>

namespace cryo {
namespace engine {

class CentralAgent;  // Forward declaration

/**
 * Base interface for all engine modules
 */
class IModule {
public:
    virtual ~IModule() = default;

    /**
     * Initialize the module
     * @param agent Reference to the central agent
     * @return true if successful, false otherwise
     */
    virtual bool initialize(CentralAgent& agent) = 0;

    /**
     * Update the module (called every frame)
     * @param deltaTime Time since last frame (seconds)
     */
    virtual void update(float deltaTime) = 0;

    /**
     * Shutdown the module
     */
    virtual void shutdown() = 0;

    /**
     * Get the module name (must be unique)
     */
    virtual std::string getName() const = 0;

    /**
     * Get the module type
     */
    virtual ModuleType getType() const = 0;

    /**
     * Get list of module dependencies (by name)
     */
    virtual std::vector<std::string> getDependencies() const = 0;

    /**
     * Handle incoming message from another module
     */
    virtual void handleMessage(const Message& msg) = 0;

    /**
     * Get module specification (for automatic documentation)
     */
    virtual ModuleSpec getSpecification() const = 0;
};

} // namespace engine
} // namespace cryo
