#pragma once

#include <string>

namespace avannaaq {

/**
 * Manages persistent game state using akutik storage backend
 */
class GameStateStore {
public:
    GameStateStore() = default;
    ~GameStateStore() = default;

    bool initialize(const std::string& dbPath);
    void shutdown();

    // State persistence
    bool saveGameState();
    bool loadGameState();

    // Player data
    bool savePlayerData(const std::string& playerId);
    bool loadPlayerData(const std::string& playerId);

    // World data
    bool saveWorldState(const std::string& worldId);
    bool loadWorldState(const std::string& worldId);

private:
    std::string m_dbPath;
    bool m_initialized = false;

    // TODO: Add akutik storage instance
};

} // namespace avannaaq
