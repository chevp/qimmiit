#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace avannaaq {

struct PlayerData {
    std::string playerId;
    std::string name;
    float x, y, z;
    int health;
    int maxHealth;
};

/**
 * Manages all connected players
 */
class PlayerManager {
public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    void update(double deltaTime);

    // Player lifecycle
    bool addPlayer(const std::string& playerId, const std::string& name);
    bool removePlayer(const std::string& playerId);

    // Player queries
    PlayerData* getPlayer(const std::string& playerId);
    size_t getPlayerCount() const { return m_players.size(); }

private:
    std::unordered_map<std::string, PlayerData> m_players;
};

} // namespace avannaaq
