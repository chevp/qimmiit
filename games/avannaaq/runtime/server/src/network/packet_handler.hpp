#pragma once

#include <vector>
#include <cstdint>

namespace avannaaq {

enum class PacketType : uint16_t {
    Connect = 1,
    Disconnect = 2,
    PlayerMove = 10,
    PlayerAction = 11,
    ChatMessage = 20,
    WorldUpdate = 30,
};

/**
 * Handles packet serialization and deserialization
 */
class PacketHandler {
public:
    static bool handlePacket(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> serializePacket(PacketType type, const void* data, size_t size);
};

} // namespace avannaaq
