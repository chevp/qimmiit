#include "UmiakAgent.hpp"
#include <iostream>
#include <regex>
#include <sstream>

namespace cryo {
namespace agents {

UmiakAgent::UmiakAgent() : running(false) {
    initializeDemoEntities();
}

UmiakAgent::~UmiakAgent() {
    stop();
}

void UmiakAgent::start() {
    if (running.load()) {
        std::cerr << "UmiakAgent already running" << std::endl;
        return;
    }

    running.store(true);
    worker = std::thread(&UmiakAgent::run, this);
    std::cout << "[UmiakAgent] Started worker thread" << std::endl;
}

void UmiakAgent::stop() {
    if (!running.load()) {
        return;
    }

    std::cout << "[UmiakAgent] Stopping worker thread..." << std::endl;
    running.store(false);
    cv.notify_all();

    if (worker.joinable()) {
        worker.join();
    }

    std::cout << "[UmiakAgent] Worker thread stopped" << std::endl;
}

void UmiakAgent::sendCommand(const AgentCommand& command) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(command);
    }
    cv.notify_one();
}

EntityData* UmiakAgent::getEntityById(const std::string& entity_id) {
    std::lock_guard<std::mutex> lock(entityMutex_);
    return findEntityById(entity_id);
}

void UmiakAgent::run() {
    while (running.load()) {
        AgentCommand command;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !commandQueue.empty() || !running.load(); });

            if (!running.load()) break;

            if (commandQueue.empty()) continue;

            command = commandQueue.front();
            commandQueue.pop();
        }

        handleCommand(command);
    }
}

void UmiakAgent::handleCommand(const AgentCommand& command) {
    std::lock_guard<std::mutex> lock(entityMutex_);

    std::visit([this](auto&& cmd) {
        using T = std::decay_t<decltype(cmd)>;
        if constexpr (std::is_same_v<T, MoveEntityRequest>) {
            handleMoveEntity(cmd);
        } else if constexpr (std::is_same_v<T, RotateEntityRequest>) {
            handleRotateEntity(cmd);
        } else if constexpr (std::is_same_v<T, DeleteEntityRequest>) {
            handleDeleteEntity(cmd);
        } else if constexpr (std::is_same_v<T, FindEntityRequest>) {
            handleFindEntity(cmd);
        }
    }, command);
}

void UmiakAgent::handleMoveEntity(const MoveEntityRequest& request) {
    std::cout << "[UmiakAgent] Moving entity '" << request.entity_id
              << "' to position (" << request.x << ", "
              << request.y << ", " << request.z << ")" << std::endl;

    EntityData* entity = findEntityById(request.entity_id);
    if (!entity) {
        std::cerr << "[UmiakAgent] Entity not found: " << request.entity_id << std::endl;
        return;
    }

    if (request.relative) {
        entity->position.x += request.x;
        entity->position.y += request.y;
        entity->position.z += request.z;
    } else {
        entity->position.x = request.x;
        entity->position.y = request.y;
        entity->position.z = request.z;
    }

    sceneDirty_ = true;

    std::cout << "[UmiakAgent] Entity '" << entity->name
              << "' moved to (" << entity->position.x << ", "
              << entity->position.y << ", " << entity->position.z << ")" << std::endl;
}

void UmiakAgent::handleRotateEntity(const RotateEntityRequest& request) {
    std::cout << "[UmiakAgent] Rotating entity '" << request.entity_id << "'" << std::endl;

    EntityData* entity = findEntityById(request.entity_id);
    if (!entity) {
        std::cerr << "[UmiakAgent] Entity not found: " << request.entity_id << std::endl;
        return;
    }

    if (request.relative) {
        // Simplified relative rotation (should use quaternion multiplication)
        entity->rotation.x += request.rotation.x;
        entity->rotation.y += request.rotation.y;
        entity->rotation.z += request.rotation.z;
        entity->rotation.w += request.rotation.w;
    } else {
        entity->rotation.x = request.rotation.x;
        entity->rotation.y = request.rotation.y;
        entity->rotation.z = request.rotation.z;
        entity->rotation.w = request.rotation.w;
    }

    sceneDirty_ = true;

    std::cout << "[UmiakAgent] Entity '" << entity->name << "' rotated" << std::endl;
}

void UmiakAgent::handleDeleteEntity(const DeleteEntityRequest& request) {
    std::cout << "[UmiakAgent] Deleting entity '" << request.entity_id << "'" << std::endl;

    auto it = entities_.find(request.entity_id);
    if (it == entities_.end()) {
        std::cerr << "[UmiakAgent] Entity not found: " << request.entity_id << std::endl;
        return;
    }

    std::string name = it->second.name;
    entities_.erase(it);
    sceneDirty_ = true;

    std::cout << "[UmiakAgent] Entity '" << name << "' deleted" << std::endl;
}

void UmiakAgent::handleFindEntity(const FindEntityRequest& request) {
    std::cout << "[UmiakAgent] Finding entity with pattern '"
              << request.name_pattern << "'" << std::endl;

    EntityData* entity = findEntityByName(request.name_pattern);
    if (entity) {
        std::cout << "[UmiakAgent] Found entity: " << entity->name
                  << " (ID: " << entity->id << ")"
                  << " at position (" << entity->position.x << ", "
                  << entity->position.y << ", " << entity->position.z << ")" << std::endl;
    } else {
        std::cout << "[UmiakAgent] Entity not found" << std::endl;
    }
}

EntityData* UmiakAgent::findEntityById(const std::string& id) {
    auto it = entities_.find(id);
    if (it != entities_.end()) {
        return &it->second;
    }
    return nullptr;
}

EntityData* UmiakAgent::findEntityByName(const std::string& name_pattern) {
    try {
        std::regex pattern(name_pattern, std::regex_constants::icase);

        for (auto& pair : entities_) {
            if (std::regex_search(pair.second.name, pattern)) {
                return &pair.second;
            }
        }
    } catch (const std::regex_error& e) {
        // If regex fails, try simple substring match
        for (auto& pair : entities_) {
            if (pair.second.name.find(name_pattern) != std::string::npos) {
                return &pair.second;
            }
        }
    }

    return nullptr;
}

void UmiakAgent::initializeDemoEntities() {
    // Demo entity 1: suzanne-in-box
    EntityData suzanne;
    suzanne.id = "entity_001";
    suzanne.name = "suzanne-in-box";
    suzanne.type = "Mesh";
    suzanne.position = {0.0f, 0.0f, 0.0f};
    suzanne.rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    suzanne.tags = {"mesh", "character"};
    entities_[suzanne.id] = suzanne;

    // Demo entity 2: main-light
    EntityData light;
    light.id = "entity_002";
    light.name = "main-light";
    light.type = "Light";
    light.position = {5.0f, 5.0f, 5.0f};
    light.rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    light.tags = {"light", "sun"};
    entities_[light.id] = light;

    // Demo entity 3: main-camera
    EntityData camera;
    camera.id = "entity_003";
    camera.name = "main-camera";
    camera.type = "Camera";
    camera.position = {0.0f, 0.0f, 10.0f};
    camera.rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    camera.tags = {"camera", "main"};
    entities_[camera.id] = camera;

    std::cout << "[UmiakAgent] Initialized " << entities_.size() << " demo entities" << std::endl;
}

} // namespace agents
} // namespace cryo
