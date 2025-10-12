#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace coregfx {
namespace platform {

struct InputState {
    struct MouseButtons {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

    struct CameraKeys {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
    } cameraKeys;

    glm::vec2 mousePos = glm::vec2(0.0f);
    bool paused = false;
};

class InputHandler {
public:
    InputHandler() = default;
    ~InputHandler() = default;

    // Mouse input
    void setMousePosition(float x, float y);
    void setMouseButton(int button, bool pressed);
    void handleMouseWheel(float delta);

    // Keyboard input
    void setCameraKey(int key, bool pressed);
    void handleSpecialKey(int key);

    // Gamepad input
    void updateGamepadInput(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime);

    // State access
    const InputState& getInputState() const { return inputState; }
    InputState& getInputState() { return inputState; }

    // Camera movement utilities
    static glm::vec3 calculateCameraMovement(const InputState& state, const glm::vec3& cameraFront,
                                            float moveSpeed, float deltaTime);
    static void handleMouseDrag(glm::vec2 currentPos, glm::vec2 lastPos, glm::vec3& rotation,
                               float rotationSpeed);

    // Platform-specific key mapping
    enum class Key {
        W, A, S, D,
        P, ESCAPE,
        UNKNOWN
    };

    static Key mapPlatformKey(int platformKey);

private:
    InputState inputState;
    glm::vec2 lastMousePos = glm::vec2(0.0f);
    bool mouseDragging = false;
};

}} // namespace coregfx::platform