#pragma once

#include <string>
#include <vector>

namespace avannaaq {
namespace cms {

struct ValidationResult {
    bool success = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/**
 * Validates game content files
 */
class ContentValidator {
public:
    ContentValidator() = default;

    // Validate individual files
    ValidationResult validateWorld(const std::string& filePath);
    ValidationResult validateItems(const std::string& filePath);
    ValidationResult validateCharacters(const std::string& filePath);

    // Validate entire content directory
    ValidationResult validateAllContent(const std::string& contentRoot);

private:
    bool fileExists(const std::string& path);
    std::string readFile(const std::string& path);
};

} // namespace cms
} // namespace avannaaq
