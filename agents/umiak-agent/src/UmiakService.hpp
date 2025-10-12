/**
 * Umiak Service Implementation
 *
 * Implements the gRPC service for scene manipulation.
 * This is where the actual work happens when requests come in.
 */

#pragma once

#include <grpcpp/grpcpp.h>
#include "umiak.grpc.pb.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <regex>

namespace cryo {
namespace agents {
namespace umiak {

// Internal data structures (NOT protobuf - real scene data!)
struct Position {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Rotation {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;  // Quaternion
};

struct EntityData {
    std::string id;
    std::string name;
    std::string type;  // "Mesh", "Light", "Camera", etc.
    Position position;
    Rotation rotation;
    std::vector<std::string> tags;
    std::string parentId;
    std::vector<std::string> childrenIds;
};

/**
 * Umiak Service - implements the gRPC service
 */
class UmiakService final : public ::cryo::agents::umiak::UmiakAgent::Service {
public:
    UmiakService();
    ~UmiakService();

    // Load/save scene
    bool loadScene(const std::string& path);
    bool saveScene(const std::string& path);

    // gRPC service methods (implement the protobuf service)
    grpc::Status findEntity(
        grpc::ServerContext* context,
        const EntityQuery* request,
        EntityResult* response) override;

    grpc::Status moveEntity(
        grpc::ServerContext* context,
        const MoveEntityRequest* request,
        OperationResult* response) override;

    grpc::Status rotateEntity(
        grpc::ServerContext* context,
        const RotateEntityRequest* request,
        OperationResult* response) override;

    grpc::Status deleteEntity(
        grpc::ServerContext* context,
        const EntityId* request,
        OperationResult* response) override;

    grpc::Status listEntities(
        grpc::ServerContext* context,
        const ListEntitiesRequest* request,
        EntityList* response) override;

    grpc::Status analyzeScene(
        grpc::ServerContext* context,
        const SceneData* request,
        SceneAnalysis* response) override;

private:
    // Helper methods
    EntityData* findEntityById(const std::string& id);
    std::vector<EntityData*> findEntitiesByPattern(const std::string& pattern);
    void fillEntityInfo(const EntityData& entity, EntityInfo* info);

    // The actual scene data lives here!
    std::map<std::string, EntityData> entities_;
    std::string currentScenePath_;
    bool sceneDirty_ = false;
};

} // namespace umiak
} // namespace agents
} // namespace cryo
