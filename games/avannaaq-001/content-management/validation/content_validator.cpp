#include "content_validator.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <google/protobuf/text_format.h>

// TODO: Include generated proto headers
// #include "world.pb.h"
// #include "items.pb.h"
// #include "characters.pb.h"

namespace avannaaq {
namespace cms {

ValidationResult ContentValidator::validateWorld(const std::string& filePath) {
    ValidationResult result;

    if (!fileExists(filePath)) {
        result.errors.push_back("File not found: " + filePath);
        return result;
    }

    // TODO: Parse and validate world proto
    std::cout << "Validating world: " << filePath << "\n";

    result.success = true;
    return result;
}

ValidationResult ContentValidator::validateItems(const std::string& filePath) {
    ValidationResult result;

    if (!fileExists(filePath)) {
        result.errors.push_back("File not found: " + filePath);
        return result;
    }

    // TODO: Parse and validate items proto
    std::cout << "Validating items: " << filePath << "\n";

    result.success = true;
    return result;
}

ValidationResult ContentValidator::validateCharacters(const std::string& filePath) {
    ValidationResult result;

    if (!fileExists(filePath)) {
        result.errors.push_back("File not found: " + filePath);
        return result;
    }

    // TODO: Parse and validate characters proto
    std::cout << "Validating characters: " << filePath << "\n";

    result.success = true;
    return result;
}

ValidationResult ContentValidator::validateAllContent(const std::string& contentRoot) {
    ValidationResult result;
    result.success = true;

    std::cout << "Validating all content in: " << contentRoot << "\n";

    // Validate worlds
    std::filesystem::path worldsPath = std::filesystem::path(contentRoot) / "worlds";
    if (std::filesystem::exists(worldsPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(worldsPath)) {
            if (entry.path().extension() == ".pbtxt") {
                auto worldResult = validateWorld(entry.path().string());
                if (!worldResult.success) {
                    result.success = false;
                    result.errors.insert(result.errors.end(),
                                       worldResult.errors.begin(),
                                       worldResult.errors.end());
                }
            }
        }
    }

    // Validate items
    std::filesystem::path itemsPath = std::filesystem::path(contentRoot) / "items";
    if (std::filesystem::exists(itemsPath)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(itemsPath)) {
            if (entry.path().extension() == ".pbtxt") {
                auto itemsResult = validateItems(entry.path().string());
                if (!itemsResult.success) {
                    result.success = false;
                    result.errors.insert(result.errors.end(),
                                       itemsResult.errors.begin(),
                                       itemsResult.errors.end());
                }
            }
        }
    }

    // Validate characters
    std::filesystem::path charactersPath = std::filesystem::path(contentRoot) / "characters";
    if (std::filesystem::exists(charactersPath)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(charactersPath)) {
            if (entry.path().extension() == ".pbtxt") {
                auto charResult = validateCharacters(entry.path().string());
                if (!charResult.success) {
                    result.success = false;
                    result.errors.insert(result.errors.end(),
                                       charResult.errors.begin(),
                                       charResult.errors.end());
                }
            }
        }
    }

    return result;
}

bool ContentValidator::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string ContentValidator::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

} // namespace cms
} // namespace avannaaq
