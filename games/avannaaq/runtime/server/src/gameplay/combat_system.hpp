#pragma once

namespace avannaaq {

/**
 * Handles combat mechanics and damage calculations
 */
class CombatSystem {
public:
    CombatSystem() = default;
    ~CombatSystem() = default;

    void update(double deltaTime);

    // Combat actions
    bool processDamage(const std::string& attackerId, const std::string& targetId, int damage);
    bool processHeal(const std::string& targetId, int amount);

private:
    // TODO: Combat state tracking
};

} // namespace avannaaq
