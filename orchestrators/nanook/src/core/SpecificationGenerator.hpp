/**
 * Cryo Engine - Specification Generator
 *
 * Automatically generates comprehensive documentation by introspecting
 * all registered modules, events, configuration, and runtime state.
 */

#pragma once

#include "IModule.hpp"
#include "ModuleSpec.hpp"
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>

namespace cryo {
namespace engine {

/**
 * Output format for specification
 */
enum class SpecFormat {
    MARKDOWN,
    JSON,
    YAML,
    HTML
};

/**
 * Engine information for specification
 */
struct EngineInfo {
    std::string version;
    std::string buildDate;
    std::string buildTime;
    std::string buildConfig;
    std::string platform;
    std::string compiler;
    size_t moduleCount;
    bool running;
    uint64_t uptime;  // seconds
};

/**
 * Event bus specification
 */
struct EventBusSpec {
    struct EventTypeInfo {
        std::string name;
        std::vector<std::string> publishers;
        std::vector<std::string> subscribers;
        std::string description;
    };

    std::vector<EventTypeInfo> eventTypes;
    size_t totalEvents;
    std::string eventFlowDiagram;
};

/**
 * Configuration specification
 */
struct ConfigSpec {
    struct Section {
        std::string name;
        std::vector<ConfigOption> options;
    };

    std::vector<Section> sections;
    size_t totalOptions;
    std::string sourceFile;
};

/**
 * Task scheduler specification
 */
struct TaskSchedulerSpec {
    size_t workerThreads;
    size_t pendingTasks;
    uint64_t completedTasks;
    uint64_t failedTasks;
    float avgTaskTimeMs;

    struct PriorityQueueInfo {
        std::string priority;
        size_t taskCount;
    };
    std::vector<PriorityQueueInfo> priorityQueues;
};

/**
 * Memory manager specification
 */
struct MemoryManagerSpec {
    size_t poolSizeMB;
    size_t allocatedMB;
    size_t peakAllocatedMB;
    size_t activeAllocations;
    uint64_t totalAllocations;
    uint64_t totalFrees;
    float utilizationPercent;
};

/**
 * Specification generator
 *
 * Collects information from all engine components and generates
 * comprehensive documentation in various formats.
 */
class SpecificationGenerator {
public:
    SpecificationGenerator() = default;
    ~SpecificationGenerator() = default;

    // Add specification sections
    void setEngineInfo(const EngineInfo& info);
    void addModule(const ModuleSpec& spec);
    void setEventBusSpec(const EventBusSpec& spec);
    void setConfigSpec(const ConfigSpec& spec);
    void setTaskSchedulerSpec(const TaskSchedulerSpec& spec);
    void setMemoryManagerSpec(const MemoryManagerSpec& spec);

    // Generate output in different formats
    std::string generateMarkdown() const;
    std::string generateJSON() const;
    std::string generateYAML() const;
    std::string generateHTML() const;

    // Write to file
    bool writeToFile(const std::string& filename, SpecFormat format) const;

private:
    EngineInfo engineInfo_;
    std::vector<ModuleSpec> modules_;
    EventBusSpec eventBusSpec_;
    ConfigSpec configSpec_;
    TaskSchedulerSpec taskSchedulerSpec_;
    MemoryManagerSpec memoryManagerSpec_;

    // Markdown generation helpers
    std::string generateMarkdownEngineInfo() const;
    std::string generateMarkdownModules() const;
    std::string generateMarkdownModule(const ModuleSpec& spec, size_t index) const;
    std::string generateMarkdownEventBus() const;
    std::string generateMarkdownConfig() const;
    std::string generateMarkdownTaskScheduler() const;
    std::string generateMarkdownMemoryManager() const;
    std::string generateMarkdownCommandLine() const;

    // JSON generation helpers
    std::string generateJSONEngineInfo() const;
    std::string generateJSONModules() const;
    std::string generateJSONModule(const ModuleSpec& spec) const;
    std::string generateJSONEventBus() const;
    std::string generateJSONConfig() const;

    // YAML generation helpers
    std::string generateYAMLEngineInfo() const;
    std::string generateYAMLModules() const;
    std::string generateYAMLModule(const ModuleSpec& spec) const;

    // HTML generation helpers
    std::string generateHTMLHeader() const;
    std::string generateHTMLEngineInfo() const;
    std::string generateHTMLModules() const;
    std::string generateHTMLModule(const ModuleSpec& spec, size_t index) const;
    std::string generateHTMLFooter() const;

    // Utility methods
    std::string escapeJSON(const std::string& str) const;
    std::string escapeHTML(const std::string& str) const;
    std::string getCurrentTimestamp() const;
    std::string moduleTypeToString(ModuleType type) const;
};

} // namespace engine
} // namespace cryo