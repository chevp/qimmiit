#include "game_loop.hpp"
#include "gameplay/world_manager.hpp"
#include "gameplay/player_manager.hpp"
#include "gameplay/combat_system.hpp"

#include <iostream>

namespace avannaaq {

GameLoop::GameLoop() = default;
GameLoop::~GameLoop() = default;

bool GameLoop::initialize() {
    // Initialize world manager
    m_worldManager = std::make_unique<WorldManager>();
    if (!m_worldManager->loadWorld("overworld")) {
        std::cerr << "Failed to load world\n";
        return false;
    }

    // Initialize player manager
    m_playerManager = std::make_unique<PlayerManager>();

    // Initialize combat system
    m_combatSystem = std::make_unique<CombatSystem>();

    return true;
}

void GameLoop::update(double deltaTime) {
    m_totalTime += deltaTime;

    // Update all gameplay systems
    if (m_worldManager) {
        m_worldManager->update(deltaTime);
    }

    if (m_playerManager) {
        m_playerManager->update(deltaTime);
    }

    if (m_combatSystem) {
        m_combatSystem->update(deltaTime);
    }
}

void GameLoop::shutdown() {
    m_combatSystem.reset();
    m_playerManager.reset();
    m_worldManager.reset();
}

} // namespace avannaaq
