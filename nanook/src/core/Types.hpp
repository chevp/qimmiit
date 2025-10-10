/**
 * Cryo Engine - Common Type Definitions
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace cryo {
namespace engine {

// Forward declarations
class IModule;
class CentralAgent;

/**
 * Module type enumeration
 */
enum class ModuleType {
    RENDERER,
    CACHE,
    ASSET,
    HTTP,
    COMPUTE,
    CUSTOM
};

/**
 * Event type enumeration
 */
enum class EventType {
    SCENE_LOADED,
    ASSET_READY,
    ASSET_FAILED,
    CACHE_HIT,
    CACHE_MISS,
    HTTP_REQUEST,
    HTTP_RESPONSE,
    FRAME_RENDERED,
    TASK_COMPLETED,
    TASK_FAILED,
    CUSTOM
};

/**
 * Message type enumeration
 */
enum class MessageType {
    LOAD_SCENE,
    SET_CAMERA,
    TAKE_SCREENSHOT,
    CACHE_REQUEST,
    CACHE_RESPONSE,
    ASSET_PROCESS,
    HTTP_REQUEST,
    CUSTOM
};

/**
 * Task priority enumeration
 */
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

/**
 * Generic payload for events and messages
 */
class Payload {
public:
    Payload() = default;

    // String payload
    explicit Payload(const std::string& str) : stringValue_(str), hasString_(true) {}

    // Integer payload
    explicit Payload(int value) : intValue_(value), hasInt_(true) {}

    // Float payload
    explicit Payload(float value) : floatValue_(value), hasFloat_(true) {}

    // Get as string
    std::string toString() const {
        if (hasString_) return stringValue_;
        if (hasInt_) return std::to_string(intValue_);
        if (hasFloat_) return std::to_string(floatValue_);
        return "";
    }

    // Get as int
    int toInt() const {
        if (hasInt_) return intValue_;
        if (hasString_) return std::stoi(stringValue_);
        if (hasFloat_) return static_cast<int>(floatValue_);
        return 0;
    }

    // Get as float
    float toFloat() const {
        if (hasFloat_) return floatValue_;
        if (hasInt_) return static_cast<float>(intValue_);
        if (hasString_) return std::stof(stringValue_);
        return 0.0f;
    }

    // Check if empty
    bool isEmpty() const {
        return !hasString_ && !hasInt_ && !hasFloat_;
    }

private:
    std::string stringValue_;
    int intValue_ = 0;
    float floatValue_ = 0.0f;
    bool hasString_ = false;
    bool hasInt_ = false;
    bool hasFloat_ = false;
};

/**
 * Event structure
 */
struct Event {
    EventType type;
    std::string source;  // Module that published the event
    Payload data;
    uint64_t timestamp;  // Milliseconds since epoch
};

/**
 * Message structure
 */
struct Message {
    MessageType type;
    std::string source;      // Module that sent the message
    std::string target;      // Module to receive the message
    Payload payload;
    uint64_t timestamp;
    uint64_t requestId = 0;  // For request/response correlation
};

/**
 * Task function type
 */
using Task = std::function<void()>;

/**
 * Event callback type
 */
using EventCallback = std::function<void(const Event&)>;

/**
 * Module name to string
 */
inline const char* moduleTypeToString(ModuleType type) {
    switch (type) {
        case ModuleType::RENDERER: return "RENDERER";
        case ModuleType::CACHE: return "CACHE";
        case ModuleType::ASSET: return "ASSET";
        case ModuleType::HTTP: return "HTTP";
        case ModuleType::COMPUTE: return "COMPUTE";
        case ModuleType::CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

/**
 * Event type to string
 */
inline const char* eventTypeToString(EventType type) {
    switch (type) {
        case EventType::SCENE_LOADED: return "SCENE_LOADED";
        case EventType::ASSET_READY: return "ASSET_READY";
        case EventType::ASSET_FAILED: return "ASSET_FAILED";
        case EventType::CACHE_HIT: return "CACHE_HIT";
        case EventType::CACHE_MISS: return "CACHE_MISS";
        case EventType::HTTP_REQUEST: return "HTTP_REQUEST";
        case EventType::HTTP_RESPONSE: return "HTTP_RESPONSE";
        case EventType::FRAME_RENDERED: return "FRAME_RENDERED";
        case EventType::TASK_COMPLETED: return "TASK_COMPLETED";
        case EventType::TASK_FAILED: return "TASK_FAILED";
        case EventType::CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

} // namespace engine
} // namespace cryo