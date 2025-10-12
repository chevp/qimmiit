/**
 * Umiak Service Implementation
 *
 * This is where the ACTUAL WORK happens.
 * When a gRPC request comes in, this code processes it.
 */

#include "UmiakService.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace cryo {
namespace agents {
namespace umiak {

UmiakService::UmiakService() {
    std::cout << "[Umiak] Service initialized\n";

    // Create some demo entities for testing
    EntityData suzanne;
    suzanne.id = "node_001";
    suzanne.name = "suzanne-in-box";
    suzanne.type = "Mesh";
    suzanne.position = {0.0f, 0.0f, 0.0f};
    suzanne.tags = {"interactive", "test"};
    entities_[suzanne.id] = suzanne;

    EntityData light;
    light.id = "light_001";
    light.name = "main-light";
    light.type = "Light";
    light.position = {5.0f, 10.0f, 5.0f};
    entities_[light.id] = light;

    EntityData camera;
    camera.id = "camera_001";
    camera.name = "main-camera";
    camera.type = "Camera";
    camera.position = {0.0f, 2.0f, 10.0f};
    entities_[camera.id] = camera;

    std::cout << "[Umiak] Loaded " << entities_.size() << " demo entities\n";
}

UmiakService::~UmiakService() {
    std::cout << "[Umiak] Service shutting down\n";
}

bool UmiakService::loadScene(const std::string& path) {
    std::cout << "[Umiak] Loading scene from: " << path << "\n";
    currentScenePath_ = path;
    // TODO: Actually load glTF scene using tinygltf or arctic-formats
    return true;
}

bool UmiakService::saveScene(const std::string& path) {
    std::cout << "[Umiak] Saving scene to: " << path << "\n";
    // TODO: Actually save scene
    sceneDirty_ = false;
    return true;
}

// ============================================================================
// FindEntity - Search for entity by name pattern
// ============================================================================

grpc::Status UmiakService::FindEntity(
    grpc::ServerContext* context,
    const EntityQuery* request,
    EntityResult* response) {

    std::cout << "[Umiak] FindEntity request\n";
    std::cout << "[Umiak]   name_pattern: " << request->name_pattern() << "\n";

    // THIS IS THE ACTUAL WORK!
    // Search through entities by pattern
    try {
        std::regex pattern(request->name_pattern());

        for (auto& [id, entity] : entities_) {
            if (std::regex_match(entity.name, pattern)) {
                std::cout << "[Umiak]   FOUND: " << entity.name << " (id: " << id << ")\n";

                response->set_found(true);
                fillEntityInfo(entity, response->mutable_entity());
                return grpc::Status::OK;
            }
        }

        // Not found
        std::cout << "[Umiak]   NOT FOUND\n";
        response->set_found(false);
        response->set_error_message("Entity not found matching pattern: " + request->name_pattern());

    } catch (const std::regex_error& e) {
        std::cerr << "[Umiak]   ERROR: Invalid regex pattern\n";
        response->set_found(false);
        response->set_error_message("Invalid regex pattern");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid regex pattern");
    }

    return grpc::Status::OK;
}

// ============================================================================
// MoveEntity - Actually move an entity in the scene
// ============================================================================

grpc::Status UmiakService::MoveEntity(
    grpc::ServerContext* context,
    const MoveEntityRequest* request,
    OperationResult* response) {

    std::cout << "[Umiak] MoveEntity request\n";
    std::cout << "[Umiak]   entity_id: " << request->entity().node_id() << "\n";
    std::cout << "[Umiak]   position: [" << request->position().x()
              << ", " << request->position().y()
              << ", " << request->position().z() << "]\n";
    std::cout << "[Umiak]   relative: " << (request->relative() ? "yes" : "no") << "\n";

    // Find the entity
    EntityData* entity = findEntityById(request->entity().node_id());

    if (!entity) {
        std::cerr << "[Umiak]   ERROR: Entity not found\n";
        response->set_success(false);
        response->set_error_message("Entity not found: " + request->entity().node_id());
        return grpc::Status::OK;
    }

    // ACTUALLY MODIFY THE ENTITY POSITION!
    // This is the real work - changing the scene data
    Position oldPos = entity->position;

    if (request->relative()) {
        // Relative movement
        entity->position.x += request->position().x();
        entity->position.y += request->position().y();
        entity->position.z += request->position().z();
    } else {
        // Absolute movement
        entity->position.x = request->position().x();
        entity->position.y = request->position().y();
        entity->position.z = request->position().z();
    }

    std::cout << "[Umiak]   MOVED!\n";
    std::cout << "[Umiak]   old: [" << oldPos.x << ", " << oldPos.y << ", " << oldPos.z << "]\n";
    std::cout << "[Umiak]   new: [" << entity->position.x << ", " << entity->position.y << ", " << entity->position.z << "]\n";

    // Mark scene as modified
    sceneDirty_ = true;

    // Build response
    response->set_success(true);
    fillEntityInfo(*entity, response->mutable_updated_entity());

    return grpc::Status::OK;
}

// ============================================================================
// RotateEntity - Rotate an entity
// ============================================================================

grpc::Status UmiakService::RotateEntity(
    grpc::ServerContext* context,
    const RotateEntityRequest* request,
    OperationResult* response) {

    std::cout << "[Umiak] RotateEntity request\n";

    EntityData* entity = findEntityById(request->entity().node_id());

    if (!entity) {
        response->set_success(false);
        response->set_error_message("Entity not found");
        return grpc::Status::OK;
    }

    // Apply rotation
    if (request->relative()) {
        // TODO: Quaternion multiplication for relative rotation
        std::cout << "[Umiak]   Warning: Relative rotation not fully implemented\n";
    } else {
        entity->rotation.x = request->rotation().x();
        entity->rotation.y = request->rotation().y();
        entity->rotation.z = request->rotation().z();
        entity->rotation.w = request->rotation().w();
    }

    sceneDirty_ = true;
    response->set_success(true);
    fillEntityInfo(*entity, response->mutable_updated_entity());

    return grpc::Status::OK;
}

// ============================================================================
// DeleteEntity - Remove an entity from the scene
// ============================================================================

grpc::Status UmiakService::DeleteEntity(
    grpc::ServerContext* context,
    const EntityId* request,
    OperationResult* response) {

    std::cout << "[Umiak] DeleteEntity request: " << request->node_id() << "\n";

    auto it = entities_.find(request->node_id());
    if (it == entities_.end()) {
        response->set_success(false);
        response->set_error_message("Entity not found");
        return grpc::Status::OK;
    }

    std::cout << "[Umiak]   DELETED: " << it->second.name << "\n";
    entities_.erase(it);
    sceneDirty_ = true;

    response->set_success(true);
    return grpc::Status::OK;
}

// ============================================================================
// ListEntities - List all entities (with optional filter)
// ============================================================================

grpc::Status UmiakService::ListEntities(
    grpc::ServerContext* context,
    const ListEntitiesRequest* request,
    EntityList* response) {

    std::cout << "[Umiak] ListEntities request\n";

    int count = 0;
    int limit = request->limit() > 0 ? request->limit() : INT_MAX;

    for (const auto& [id, entity] : entities_) {
        // Apply filters if present
        bool matches = true;

        if (request->has_filter()) {
            const auto& filter = request->filter();

            if (!filter.type().empty() && entity.type != filter.type()) {
                matches = false;
            }

            if (!filter.name_pattern().empty()) {
                std::regex pattern(filter.name_pattern());
                if (!std::regex_match(entity.name, pattern)) {
                    matches = false;
                }
            }
        }

        if (matches) {
            if (count < limit) {
                fillEntityInfo(entity, response->add_entities());
                count++;
            }
        }
    }

    response->set_total_count(count);
    std::cout << "[Umiak]   Found " << count << " entities\n";

    return grpc::Status::OK;
}

// ============================================================================
// AnalyzeScene - Provide scene statistics and warnings
// ============================================================================

grpc::Status UmiakService::AnalyzeScene(
    grpc::ServerContext* context,
    const SceneData* request,
    SceneAnalysis* response) {

    std::cout << "[Umiak] AnalyzeScene request\n";

    // Count entity types
    int meshCount = 0;
    int lightCount = 0;

    for (const auto& [id, entity] : entities_) {
        if (entity.type == "Mesh") meshCount++;
        else if (entity.type == "Light") lightCount++;
    }

    response->set_entity_count(entities_.size());
    response->set_mesh_count(meshCount);
    response->set_light_count(lightCount);

    // Generate warnings
    if (lightCount == 0) {
        response->add_warnings("No lights in scene - will be very dark!");
    }

    if (meshCount == 0) {
        response->add_warnings("No meshes in scene - nothing to render!");
    }

    std::cout << "[Umiak]   Entities: " << entities_.size() << "\n";
    std::cout << "[Umiak]   Meshes: " << meshCount << "\n";
    std::cout << "[Umiak]   Lights: " << lightCount << "\n";
    std::cout << "[Umiak]   Warnings: " << response->warnings_size() << "\n";

    return grpc::Status::OK;
}

// ============================================================================
// Helper Methods
// ============================================================================

EntityData* UmiakService::findEntityById(const std::string& id) {
    auto it = entities_.find(id);
    return it != entities_.end() ? &it->second : nullptr;
}

std::vector<EntityData*> UmiakService::findEntitiesByPattern(const std::string& pattern) {
    std::vector<EntityData*> results;
    std::regex regex(pattern);

    for (auto& [id, entity] : entities_) {
        if (std::regex_match(entity.name, regex)) {
            results.push_back(&entity);
        }
    }

    return results;
}

void UmiakService::fillEntityInfo(const EntityData& entity, EntityInfo* info) {
    info->set_node_id(entity.id);
    info->set_name(entity.name);
    info->set_type(entity.type);

    auto* transform = info->mutable_transform();
    transform->mutable_position()->set_x(entity.position.x);
    transform->mutable_position()->set_y(entity.position.y);
    transform->mutable_position()->set_z(entity.position.z);

    transform->mutable_rotation()->set_x(entity.rotation.x);
    transform->mutable_rotation()->set_y(entity.rotation.y);
    transform->mutable_rotation()->set_z(entity.rotation.z);
    transform->mutable_rotation()->set_w(entity.rotation.w);

    for (const auto& tag : entity.tags) {
        info->add_tags(tag);
    }
}

} // namespace umiak
} // namespace agents
} // namespace cryo
