#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <type_traits>
#include <cstring>

namespace akutik {
namespace tools {

/**
 * @brief Ulu - Data slicing and serialization tool
 *
 * Named after the traditional Inuit crescent-shaped knife (ᐅᓗ),
 * used for precise cutting. Ulu slices data into storable pieces
 * and reassembles them.
 *
 * Responsibilities:
 * - Serialize data to binary format
 * - Deserialize binary data back to objects
 * - Split large data into chunks
 * - Merge chunks back together
 *
 * No business logic - pure data transformation utility.
 */
class Ulu {
public:
    /**
     * @brief Serialize a trivially copyable type to bytes
     * @tparam T Type to serialize (must be trivially copyable)
     * @param value Value to serialize
     * @return Binary representation
     */
    template<typename T>
    static std::vector<uint8_t> serialize(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "Type must be trivially copyable");

        std::vector<uint8_t> data(sizeof(T));
        std::memcpy(data.data(), &value, sizeof(T));
        return data;
    }

    /**
     * @brief Serialize a string to bytes
     * @param str String to serialize
     * @return Binary representation
     */
    static std::vector<uint8_t> serialize(const std::string& str);

    /**
     * @brief Deserialize bytes to a trivially copyable type
     * @tparam T Type to deserialize to
     * @param data Binary data
     * @return Deserialized value, or default if data is invalid
     */
    template<typename T>
    static T deserialize(const std::vector<uint8_t>& data) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "Type must be trivially copyable");

        if (data.size() != sizeof(T)) {
            return T{}; // Return default value on size mismatch
        }

        T value;
        std::memcpy(&value, data.data(), sizeof(T));
        return value;
    }

    /**
     * @brief Deserialize bytes to a string
     * @param data Binary data
     * @return Deserialized string
     */
    static std::string deserializeString(const std::vector<uint8_t>& data);

    /**
     * @brief Slice data into fixed-size chunks
     * @param data Data to slice
     * @param chunkSize Size of each chunk in bytes
     * @return Vector of chunks
     */
    static std::vector<std::vector<uint8_t>> slice(
        const std::vector<uint8_t>& data,
        size_t chunkSize
    );

    /**
     * @brief Merge chunks back into a single data block
     * @param chunks Vector of data chunks
     * @return Merged data
     */
    static std::vector<uint8_t> merge(
        const std::vector<std::vector<uint8_t>>& chunks
    );

    /**
     * @brief Calculate checksum for data integrity
     * @param data Data to checksum
     * @return 32-bit checksum
     */
    static uint32_t checksum(const std::vector<uint8_t>& data);

    /**
     * @brief Verify data integrity
     * @param data Data to verify
     * @param expectedChecksum Expected checksum value
     * @return true if checksum matches
     */
    static bool verify(const std::vector<uint8_t>& data, uint32_t expectedChecksum);
};

} // namespace tools
} // namespace akutik
