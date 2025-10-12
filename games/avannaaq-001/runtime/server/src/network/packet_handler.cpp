#include "packet_handler.hpp"
#include <iostream>
#include <cstring>

namespace avannaaq {

bool PacketHandler::handlePacket(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(uint16_t)) {
        return false; // Invalid packet
    }

    // Extract packet type
    uint16_t typeValue;
    std::memcpy(&typeValue, data.data(), sizeof(uint16_t));
    PacketType type = static_cast<PacketType>(typeValue);

    // TODO: Dispatch to appropriate handler based on type
    switch (type) {
        case PacketType::Connect:
            std::cout << "Received Connect packet\n";
            break;
        case PacketType::Disconnect:
            std::cout << "Received Disconnect packet\n";
            break;
        case PacketType::PlayerMove:
            std::cout << "Received PlayerMove packet\n";
            break;
        default:
            std::cout << "Received unknown packet type: " << typeValue << "\n";
            break;
    }

    return true;
}

std::vector<uint8_t> PacketHandler::serializePacket(PacketType type,
                                                     const void* data,
                                                     size_t size) {
    std::vector<uint8_t> packet;
    packet.resize(sizeof(uint16_t) + size);

    // Write packet type
    uint16_t typeValue = static_cast<uint16_t>(type);
    std::memcpy(packet.data(), &typeValue, sizeof(uint16_t));

    // Write payload
    if (data && size > 0) {
        std::memcpy(packet.data() + sizeof(uint16_t), data, size);
    }

    return packet;
}

} // namespace avannaaq
