/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

class ResourceManager {
public:
    // Public method to access the singleton instance
    static ResourceManager& getInstance() {
        static ResourceManager instance; // Guaranteed to be destroyed and instantiated on first use.
        return instance;
    }

    // Delete copy constructor and assignment operator
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Method to load a resource (JSON file in this case)
    std::string load(const std::string_view path) {
        
        auto spath = std::string(path);

        // Check if the resource is already loaded
        auto it = resourceCache.find(spath);
        if (it != resourceCache.end()) {
            return it->second;
        }

        // Load the resource from file

        std::ifstream file(spath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Cache the loaded resource
        resourceCache[spath] = content;
        return content;
    }

private:
    // Private constructor for singleton
    ResourceManager() = default;

    // Resource cache
    std::map<std::string, std::string> resourceCache;
};