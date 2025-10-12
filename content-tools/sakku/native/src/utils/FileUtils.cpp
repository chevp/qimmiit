#include <sakku/FileUtils.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace sakku {
namespace utils {

bool FileUtils::exists(const std::string& path) {
    return fs::exists(path);
}

bool FileUtils::isDirectory(const std::string& path) {
    return fs::is_directory(path);
}

bool FileUtils::createDirectory(const std::string& path) {
    try {
        return fs::create_directories(path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to create directory: " << e.what() << "\n";
        return false;
    }
}

std::vector<std::string> FileUtils::listFiles(const std::string& directory, bool recursive) {
    std::vector<std::string> files;

    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to list files: " << e.what() << "\n";
    }

    return files;
}

std::optional<size_t> FileUtils::getFileSize(const std::string& path) {
    try {
        return fs::file_size(path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to get file size: " << e.what() << "\n";
        return std::nullopt;
    }
}

std::optional<std::string> FileUtils::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << path << "\n";
        return std::nullopt;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool FileUtils::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << path << "\n";
        return false;
    }

    file << content;
    return true;
}

bool FileUtils::copyFile(const std::string& source, const std::string& destination) {
    try {
        fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to copy file: " << e.what() << "\n";
        return false;
    }
}

std::string FileUtils::getCurrentDirectory() {
    try {
        return fs::current_path().string();
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to get current directory: " << e.what() << "\n";
        return "";
    }
}

std::string FileUtils::joinPath(const std::string& base, const std::string& component) {
    fs::path p = base;
    p /= component;
    return p.string();
}

std::string FileUtils::getAbsolutePath(const std::string& path) {
    try {
        return fs::absolute(path).string();
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to get absolute path: " << e.what() << "\n";
        return path;
    }
}

std::string FileUtils::getDirectory(const std::string& path) {
    fs::path p = path;
    return p.parent_path().string();
}

std::string FileUtils::getFilename(const std::string& path) {
    fs::path p = path;
    return p.filename().string();
}

bool FileUtils::createArchive(const std::string& sourceDir, const std::string& archivePath) {
    // TODO: Implement zip archive creation
    std::cerr << "Archive creation not yet implemented\n";
    std::cerr << "Would create: " << archivePath << " from " << sourceDir << "\n";
    return false;
}

bool FileUtils::extractArchive(const std::string& archivePath, const std::string& destinationDir) {
    // TODO: Implement zip archive extraction
    std::cerr << "Archive extraction not yet implemented\n";
    std::cerr << "Would extract: " << archivePath << " to " << destinationDir << "\n";
    return false;
}

} // namespace utils
} // namespace sakku
