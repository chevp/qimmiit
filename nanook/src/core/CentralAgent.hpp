/**
 * Nanook Engine - Central Agent
 */

#pragma once

#include "IModule.hpp"
#include "ConfigManager.hpp"
#include "LogManager.hpp"
#include "EventBus.hpp"
#include "TaskScheduler.hpp"
#include "MemoryManager.hpp"
#include "SpecificationGenerator.hpp"
#include "EngineConfig.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

// Forward declare GLFW types
struct GLFWwindow;

namespace cryo {
namespace engine {

class CentralAgent {
public:
    static CentralAgent& getInstance();

    CentralAgent(const CentralAgent&) = delete;
    CentralAgent& operator=(const CentralAgent&) = delete;

    bool initialize(const EngineConfig& config);
    void shutdown();
    void update(float deltaTime);

    void registerModule(std::shared_ptr<IModule> module);
    void unregisterModule(const std::string& moduleName);

    template<typename T>
    std::shared_ptr<T> getModule(const std::string& name) {
        auto it = modules_.find(name);
        if (it != modules_.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    ConfigManager& getConfigManager() { return configManager_; }
    LogManager& getLogManager() { return logManager_; }
    EventBus& getEventBus() { return eventBus_; }
    TaskScheduler& getTaskScheduler() { return taskScheduler_; }
    MemoryManager& getMemoryManager() { return memoryManager_; }

    // Window access (provided by Nunaq)
    GLFWwindow* getWindow() const { return window_; }

    void sendMessage(const std::string& targetModule, const Message& msg);
    void broadcastMessage(const Message& msg);

    bool isRunning() const { return running_; }
    void requestShutdown() { running_ = false; }

    size_t getModuleCount() const { return modules_.size(); }
    std::vector<std::string> getModuleNames() const;
    std::shared_ptr<IModule> getModuleByName(const std::string& name) const;

    void generateSpecification(SpecFormat format, const std::string& outputFile,
                              const std::string& moduleFilter = "",
                              const std::string& sectionFilter = "");

private:
    CentralAgent() = default;
    ~CentralAgent() = default;

    bool initializeModules();
    std::vector<std::shared_ptr<IModule>> topologicalSortModules();

    bool running_ = false;
    GLFWwindow* window_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<IModule>> modules_;
    std::vector<std::shared_ptr<IModule>> moduleInitOrder_;

    ConfigManager configManager_;
    LogManager logManager_;
    EventBus eventBus_;
    TaskScheduler taskScheduler_;
    MemoryManager memoryManager_;
};

} // namespace engine
} // namespace cryo
