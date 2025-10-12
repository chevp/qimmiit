#pragma once

#include <string>
#include <cstdint>

namespace avannaaq {

/**
 * Manages network connections and packet routing
 */
class ConnectionManager {
public:
    ConnectionManager() = default;
    ~ConnectionManager() = default;

    bool initialize(uint16_t port);
    void update();
    void shutdown();

    size_t getPlayerCount() const { return m_playerCount; }

private:
    uint16_t m_port = 0;
    size_t m_playerCount = 0;
    bool m_initialized = false;
};

} // namespace avannaaq
