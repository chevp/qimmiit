#pragma once

#include <string>
#include <vector>
#include <optional>

namespace sakku {
namespace utils {

class FileUtils {
public:
    // Check if path exists
    static bool exists(const std::string& path);

    // Check if path is a directory
    static bool isDirectory(const std::string& path);

    // Create directory recursively
    static bool createDirectory(const std::string& path);

    // List files in directory
    static std::vector<std::string> listFiles(const std::string& directory, bool recursive = false);

    // Get file size
    static std::optional<size_t> getFileSize(const std::string& path);

    // Read entire file as string
    static std::optional<std::string> readFile(const std::string& path);

    // Write string to file
    static bool writeFile(const std::string& path, const std::string& content);

    // Copy file
    static bool copyFile(const std::string& source, const std::string& destination);

    // Get current working directory
    static std::string getCurrentDirectory();

    // Join path components
    static std::string joinPath(const std::string& base, const std::string& component);

    // Get absolute path
    static std::string getAbsolutePath(const std::string& path);

    // Extract directory from path
    static std::string getDirectory(const std::string& path);

    // Extract filename from path
    static std::string getFilename(const std::string& path);

    // Create archive from directory
    static bool createArchive(const std::string& sourceDir, const std::string& archivePath);

    // Extract archive to directory
    static bool extractArchive(const std::string& archivePath, const std::string& destinationDir);
};

} // namespace utils
} // namespace sakku