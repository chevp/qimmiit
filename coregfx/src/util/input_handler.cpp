#include <coregfx/util/input_handler.hpp>
#include <coregfx/core/ocean_keycodes.hpp>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace coregfx {
namespace platform {

void InputHandler::setMousePosition(float x, float y) {
    inputState.mousePos = glm::vec2(x, y);
}

void InputHandler::setMouseButton(int button, bool pressed) {
    switch (button) {
        case 0: // Left button
            inputState.mouseButtons.left = pressed;
            break;
        case 1: // Right button
            inputState.mouseButtons.right = pressed;
            break;
        case 2: // Middle button
            inputState.mouseButtons.middle = pressed;
            break;
    }
}

void InputHandler::handleMouseWheel(float delta) {
    // Mouse wheel delta handling - would be connected to camera zoom
    // Implementation depends on camera interface
}

void InputHandler::setCameraKey(int key, bool pressed) {
    Key mappedKey = mapPlatformKey(key);

    switch (mappedKey) {
        case Key::W:
            inputState.cameraKeys.up = pressed;
            break;
        case Key::S:
            inputState.cameraKeys.down = pressed;
            break;
        case Key::A:
            inputState.cameraKeys.left = pressed;
            break;
        case Key::D:
            inputState.cameraKeys.right = pressed;
            break;
        default:
            break;
    }
}

void InputHandler::handleSpecialKey(int key) {
    Key mappedKey = mapPlatformKey(key);

    switch (mappedKey) {
        case Key::P:
            inputState.paused = !inputState.paused;
            break;
        case Key::ESCAPE:
            // Handle escape - would signal application exit
            break;
        default:
            break;
    }
}

void InputHandler::updateGamepadInput(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime) {
    const float deadZone = 0.0015f;
    const float range = 1.0f - deadZone;

    // Process left stick (movement)
    if (std::abs(axisLeft.x) > deadZone || std::abs(axisLeft.y) > deadZone) {
        // Left stick controls movement
        inputState.cameraKeys.up = axisLeft.y > deadZone;
        inputState.cameraKeys.down = axisLeft.y < -deadZone;
        inputState.cameraKeys.left = axisLeft.x < -deadZone;
        inputState.cameraKeys.right = axisLeft.x > deadZone;
    } else {
        // Clear movement when stick is in deadzone
        inputState.cameraKeys.up = false;
        inputState.cameraKeys.down = false;
        inputState.cameraKeys.left = false;
        inputState.cameraKeys.right = false;
    }

    // Process right stick (camera rotation)
    if (std::abs(axisRight.x) > deadZone || std::abs(axisRight.y) > deadZone) {
        // Right stick controls camera rotation
        // Implementation would update camera rotation based on axis values
    }
}

glm::vec3 InputHandler::calculateCameraMovement(const InputState& state, const glm::vec3& cameraFront,
                                               float moveSpeed, float deltaTime) {
    glm::vec3 movement(0.0f);

    if (state.cameraKeys.up) {
        movement += cameraFront;
    }
    if (state.cameraKeys.down) {
        movement -= cameraFront;
    }
    if (state.cameraKeys.left) {
        movement -= glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    if (state.cameraKeys.right) {
        movement += glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    return movement * moveSpeed * deltaTime;
}

void InputHandler::handleMouseDrag(glm::vec2 currentPos, glm::vec2 lastPos, glm::vec3& rotation,
                                  float rotationSpeed) {
    glm::vec2 delta = currentPos - lastPos;

    // Update rotation based on mouse movement
    rotation.y += delta.x * rotationSpeed;
    rotation.x += delta.y * rotationSpeed;

    // Clamp pitch to prevent camera flipping
    rotation.x = std::clamp(rotation.x, -89.0f, 89.0f);
}

InputHandler::Key InputHandler::mapPlatformKey(int platformKey) {
#ifdef _WIN32
    switch (platformKey) {
        case KEY_W: return Key::W;
        case KEY_A: return Key::A;
        case KEY_S: return Key::S;
        case KEY_D: return Key::D;
        case KEY_P: return Key::P;
        case KEY_ESCAPE: return Key::ESCAPE;
        default: return Key::UNKNOWN;
    }
#else
    // Platform-specific key mapping for other platforms would go here
    return Key::UNKNOWN;
#endif
}

}} // namespace coregfx::platform