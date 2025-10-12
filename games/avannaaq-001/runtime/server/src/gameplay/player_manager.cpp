#include "player_manager.hpp"
#include <iostream>

namespace avannaaq {

void PlayerManager::update(double deltaTime) {
    // TODO: Update player states, process movement, etc.
}

bool PlayerManager::addPlayer(const std::string& playerId, const std::string& name) {
    if (m_players.find(playerId) != m_players.end()) {
        return false; // Player already exists
    }

    PlayerData player;
    player.playerId = playerId;
    player.name = name;
    player.x = 0.0f;
    player.y = 100.0f;
    player.z = 0.0f;
    player.health = 100;
    player.maxHealth = 100;

    m_players[playerId] = player;

    std::cout << "Player added: " << name << " (ID: " << playerId << ")\n";
    return true;
}

bool PlayerManager::removePlayer(const std::string& playerId) {
    auto it = m_players.find(playerId);
    if (it == m_players.end()) {
        return false;
    }

    std::cout << "Player removed: " << it->second.name << "\n";
    m_players.erase(it);
    return true;
}

PlayerData* PlayerManager::getPlayer(const std::string& playerId) {
    auto it = m_players.find(playerId);
    if (it == m_players.end()) {
        return nullptr;
    }
    return &it->second;
}

} // namespace avannaaq
