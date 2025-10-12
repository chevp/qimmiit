/**
 * Cryo Engine - Central Agent Implementation
 */

#include "CentralAgent.hpp"
#include <algorithm>
#include <queue>
#include <iostream>

namespace cryo {
namespace engine {

CentralAgent& CentralAgent::getInstance() {
    static CentralAgent instance;
    return instance;
}

bool CentralAgent::initialize(const EngineConfig& config) {
    if (!config.isValid()) {
        std::cerr << "Invalid engine configuration" << std::endl;
        return false;
    }

    // Store window handle from Nunaq
    window_ = config.window;

    // Initialize global services
    if (!logManager_.initialize(config.logLevel, config.logOutputs)) {
        std::cerr << "Failed to initialize LogManager" << std::endl;
        return false;
    }

    logManager_.info("CentralAgent", "Initializing Cryo Engine...");
    if (window_) {
        logManager_.info("CentralAgent", "Window handle received from Nunaq");
    } else {
        logManager_.warn("CentralAgent", "No window handle provided (headless mode)");
    }

    // ConfigManager is optional - only initialize if config file is provided
    if (!config.configFile.empty()) {
        if (!configManager_.initialize(config.configFile)) {
            logManager_.error("CentralAgent", "Failed to load configuration");
            return false;
        }
    } else {
        logManager_.info("CentralAgent", "No config file specified, using in-memory configuration");
    }

    if (!eventBus_.initialize()) {
        logManager_.error("CentralAgent", "Failed to initialize EventBus");
        return false;
    }

    if (!taskScheduler_.initialize(config.threadCount)) {
        logManager_.error("CentralAgent", "Failed to initialize TaskScheduler");
        return false;
    }

    if (!memoryManager_.initialize(config.memoryPoolSizeMB)) {
        logManager_.error("CentralAgent", "Failed to initialize MemoryManager");
        return false;
    }

    // Initialize modules in dependency order
    if (!initializeModules()) {
        logManager_.error("CentralAgent", "Failed to initialize modules");
        return false;
    }

    running_ = true;
    logManager_.info("CentralAgent", "Cryo Engine initialized successfully");
    return true;
}

void CentralAgent::shutdown() {
    logManager_.info("CentralAgent", "Shutting down Cryo Engine...");

    running_ = false;

    // Shutdown modules in reverse order
    for (auto it = moduleInitOrder_.rbegin(); it != moduleInitOrder_.rend(); ++it) {
        logManager_.info("CentralAgent", "Shutting down module: " + (*it)->getName());
        (*it)->shutdown();
    }

    taskScheduler_.shutdown();
    memoryManager_.shutdown();
    eventBus_.shutdown();
    configManager_.shutdown();

    logManager_.info("CentralAgent", "Cryo Engine shutdown complete");
    logManager_.shutdown();
}

void CentralAgent::update(float deltaTime) {
    if (!running_) return;

    // Update all modules
    for (auto& module : moduleInitOrder_) {
        module->update(deltaTime);
    }
}

void CentralAgent::registerModule(std::shared_ptr<IModule> module) {
    if (!module) {
        logManager_.warn("CentralAgent", "Attempted to register null module");
        return;
    }

    std::string name = module->getName();
    if (modules_.find(name) != modules_.end()) {
        logManager_.warn("CentralAgent", "Module already registered: " + name);
        return;
    }

    modules_[name] = module;
    logManager_.info("CentralAgent", "Registered module: " + name);
}

void CentralAgent::unregisterModule(const std::string& moduleName) {
    auto it = modules_.find(moduleName);
    if (it != modules_.end()) {
        modules_.erase(it);
        logManager_.info("CentralAgent", "Unregistered module: " + moduleName);
    }
}

void CentralAgent::sendMessage(const std::string& targetModule, const Message& msg) {
    auto it = modules_.find(targetModule);
    if (it != modules_.end()) {
        it->second->handleMessage(msg);
    } else {
        logManager_.warn("CentralAgent", "Target module not found: " + targetModule);
    }
}

void CentralAgent::broadcastMessage(const Message& msg) {
    for (auto& [name, module] : modules_) {
        module->handleMessage(msg);
    }
}

std::vector<std::string> CentralAgent::getModuleNames() const {
    std::vector<std::string> names;
    names.reserve(modules_.size());
    for (const auto& [name, module] : modules_) {
        names.push_back(name);
    }
    return names;
}

std::shared_ptr<IModule> CentralAgent::getModuleByName(const std::string& name) const {
    auto it = modules_.find(name);
    return (it != modules_.end()) ? it->second : nullptr;
}

bool CentralAgent::initializeModules() {
    // Topological sort modules by dependencies
    moduleInitOrder_ = topologicalSortModules();

    if (moduleInitOrder_.size() != modules_.size()) {
        logManager_.error("CentralAgent", "Circular dependency detected in modules");
        return false;
    }

    // Initialize each module
    for (auto& module : moduleInitOrder_) {
        logManager_.info("CentralAgent", "Initializing module: " + module->getName());

        if (!module->initialize(*this)) {
            logManager_.error("CentralAgent", "Failed to initialize module: " + module->getName());
            return false;
        }
    }

    return true;
}

std::vector<std::shared_ptr<IModule>> CentralAgent::topologicalSortModules() {
    std::unordered_map<std::string, int> inDegree;
    std::unordered_map<std::string, std::vector<std::string>> adjList;

    // Build dependency graph
    for (const auto& [name, module] : modules_) {
        inDegree[name] = 0;
        adjList[name] = {};
    }

    for (const auto& [name, module] : modules_) {
        auto deps = module->getDependencies();
        for (const auto& dep : deps) {
            if (modules_.find(dep) != modules_.end()) {
                adjList[dep].push_back(name);
                inDegree[name]++;
            }
        }
    }

    // Find modules with no dependencies
    std::queue<std::string> queue;
    for (const auto& [name, degree] : inDegree) {
        if (degree == 0) {
            queue.push(name);
        }
    }

    // Process queue (topological sort)
    std::vector<std::shared_ptr<IModule>> sorted;
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        sorted.push_back(modules_[current]);

        for (const auto& neighbor : adjList[current]) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }

    return sorted;
}

void CentralAgent::generateSpecification(SpecFormat format, const std::string& outputFile,
                                        const std::string& moduleFilter,
                                        const std::string& sectionFilter) {
    // TODO: Implement specification generation
    logManager_.info("CentralAgent", "Generating specification (not yet implemented)");
}

} // namespace engine
} // namespace cryo