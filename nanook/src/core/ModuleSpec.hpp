/**
 * Cryo Engine - Module Specification Data Structures
 *
 * Data structures for module introspection and automatic documentation.
 */

#pragma once

#include <string>
#include <vector>
#include <map>

namespace cryo {
namespace engine {

/**
 * API method parameter
 */
struct Parameter {
    std::string name;
    std::string type;
    std::string description;
    bool required;
    std::string defaultValue;
};

/**
 * API method specification
 */
struct APIMethod {
    std::string name;
    std::string description;
    std::vector<Parameter> parameters;
    std::string returnType;
    std::string example;  // Code example
};

/**
 * Configuration option specification
 */
struct ConfigOption {
    std::string key;
    std::string type;  // "bool", "int", "float", "string", "vec3", etc.
    std::string description;
    std::string defaultValue;
    std::string validationRule;  // "true|false", "range(1,10)", etc.
    std::string currentValue;
};

/**
 * Module specification
 *
 * Each module implements IModule::getSpecification() to return this structure.
 */
struct ModuleSpec {
    // Basic info
    std::string name;
    std::string version;
    ModuleType type;
    std::string description;

    // Dependencies
    std::vector<std::string> dependencies;

    // Public API
    std::vector<APIMethod> publicMethods;

    // Configuration
    std::vector<ConfigOption> configOptions;

    // Events
    std::vector<std::string> publishedEvents;   // EventType names
    std::vector<std::string> subscribedEvents;  // EventType names

    // Runtime statistics (optional, can be empty)
    std::map<std::string, std::string> statistics;
};

} // namespace engine
} // namespace cryo