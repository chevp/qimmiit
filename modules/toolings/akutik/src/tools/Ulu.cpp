#include "akutik/tools/Ulu.hpp"
#include <algorithm>

namespace akutik {
namespace tools {

std::vector<uint8_t> Ulu::serialize(const std::string& str) {
    std::vector<uint8_t> data(str.size());
    std::memcpy(data.data(), str.data(), str.size());
    return data;
}

std::string Ulu::deserializeString(const std::vector<uint8_t>& data) {
    return std::string(reinterpret_cast<const char*>(data.data()), data.size());
}

std::vector<std::vector<uint8_t>> Ulu::slice(
    const std::vector<uint8_t>& data,
    size_t chunkSize)
{
    if (chunkSize == 0) {
        return {}; // Invalid chunk size
    }

    std::vector<std::vector<uint8_t>> chunks;
    size_t offset = 0;

    while (offset < data.size()) {
        size_t currentChunkSize = std::min(chunkSize, data.size() - offset);
        std::vector<uint8_t> chunk(currentChunkSize);
        std::memcpy(chunk.data(), data.data() + offset, currentChunkSize);
        chunks.push_back(std::move(chunk));
        offset += currentChunkSize;
    }

    return chunks;
}

std::vector<uint8_t> Ulu::merge(const std::vector<std::vector<uint8_t>>& chunks) {
    size_t totalSize = 0;
    for (const auto& chunk : chunks) {
        totalSize += chunk.size();
    }

    std::vector<uint8_t> merged;
    merged.reserve(totalSize);

    for (const auto& chunk : chunks) {
        merged.insert(merged.end(), chunk.begin(), chunk.end());
    }

    return merged;
}

uint32_t Ulu::checksum(const std::vector<uint8_t>& data) {
    // Simple CRC32-like checksum
    uint32_t crc = 0xFFFFFFFF;

    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = crc >> 1;
            }
        }
    }

    return ~crc;
}

bool Ulu::verify(const std::vector<uint8_t>& data, uint32_t expectedChecksum) {
    return checksum(data) == expectedChecksum;
}

} // namespace tools
} // namespace akutik
