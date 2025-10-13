#pragma once

#include <memory>

namespace avannaaq {

class WorldManager;
class PlayerManager;
class CombatSystem;

/**
 * Core game loop - manages all gameplay systems
 */
class GameLoop {
public:
    GameLoop();
    ~GameLoop();

    bool initialize();
    void update(double deltaTime);
    void shutdown();

private:
    std::unique_ptr<WorldManager> m_worldManager;
    std::unique_ptr<PlayerManager> m_playerManager;
    std::unique_ptr<CombatSystem> m_combatSystem;

    double m_totalTime = 0.0;
};

} // namespace avannaaq
