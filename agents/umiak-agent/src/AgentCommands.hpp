#pragma once

#include <string>
#include <variant>

namespace cryo {
namespace agents {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct MoveEntityRequest {
    std::string entity_id;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    bool relative = false;
};

struct MoveEntityResponse {
    bool success = false;
    std::string message;
};

struct RotateEntityRequest {
    std::string entity_id;
    Quat rotation;
    bool relative = false;
};

struct RotateEntityResponse {
    bool success = false;
    std::string message;
};

struct DeleteEntityRequest {
    std::string entity_id;
};

struct DeleteEntityResponse {
    bool success = false;
    std::string message;
};

struct FindEntityRequest {
    std::string name_pattern;
    std::string type;
};

struct FindEntityResponse {
    bool found = false;
    std::string entity_id;
    std::string name;
    Vec3 position;
    std::string message;
};

// Command wrapper using std::variant
using AgentCommand = std::variant<
    MoveEntityRequest,
    RotateEntityRequest,
    DeleteEntityRequest,
    FindEntityRequest
>;

} // namespace agents
} // namespace cryo
