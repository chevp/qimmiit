#pragma once

#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <map>
#include <vector>

#include "AgentCommands.hpp"

namespace cryo {
namespace agents {

struct Position {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Rotation {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct EntityData {
    std::string id;
    std::string name;
    std::string type;
    Position position;
    Rotation rotation;
    std::vector<std::string> tags;
};

/**
 * Umiak Agent - Thread-based scene manipulation agent
 *
 * Processes commands asynchronously in a dedicated thread.
 * Commands are sent via C++ structs through a command queue.
 */
class UmiakAgent {
public:
    UmiakAgent();
    ~UmiakAgent();

    // Thread control
    void start();
    void stop();

    // Command interface
    void sendCommand(const AgentCommand& command);

    // Direct query for testing (bypasses queue)
    EntityData* getEntityById(const std::string& entity_id);

private:
    // Thread management
    std::thread worker;
    std::atomic<bool> running;

    // Command queue
    std::queue<AgentCommand> commandQueue;
    std::mutex queueMutex;
    std::condition_variable cv;

    // Scene data
    std::map<std::string, EntityData> entities_;
    std::mutex entityMutex_;
    bool sceneDirty_ = false;

    // Thread function
    void run();

    // Command handlers
    void handleCommand(const AgentCommand& command);
    void handleMoveEntity(const MoveEntityRequest& request);
    void handleRotateEntity(const RotateEntityRequest& request);
    void handleDeleteEntity(const DeleteEntityRequest& request);
    void handleFindEntity(const FindEntityRequest& request);

    // Internal helpers
    EntityData* findEntityById(const std::string& id);
    EntityData* findEntityByName(const std::string& name_pattern);
    void initializeDemoEntities();
};

} // namespace agents
} // namespace cryo
