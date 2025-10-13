#pragma once

#include <string>
#include <vector>
#include <memory>

namespace avannaaq {

/**
 * Manages the game world and its regions
 */
class WorldManager {
public:
    WorldManager() = default;
    ~WorldManager() = default;

    bool loadWorld(const std::string& worldId);
    void update(double deltaTime);

    // World state queries
    bool isValidPosition(float x, float y, float z) const;
    std::string getRegionAt(float x, float y, float z) const;

private:
    std::string m_currentWorldId;
    // TODO: World data structure
};

} // namespace avannaaq
