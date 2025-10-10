/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef GLTF_LOADER_HPP_
#define GLTF_LOADER_HPP_

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <tinygltf/tiny_gltf.h>

class GltfLoader {
public:
    // Public method to access the singleton instance
    static GltfLoader& getInstance() {
        static GltfLoader instance; // Guaranteed to be destroyed and instantiated on first use.
        return instance;
    }

    // Delete copy constructor and assignment operator
    GltfLoader(const GltfLoader&) = delete;
    GltfLoader& operator=(const GltfLoader&) = delete;

    // Method to load a resource (GLTF file in this case)
    tinygltf::Model loadGltfRsc(uint32_t id, const std::string_view gltf_fullpath) {

        // Check if the resource is already loaded
        auto it = resourceCache.find(id);
        if (it != resourceCache.end()) {
            return it->second;
        }

        if (gltf_fullpath.empty()) {
            std::cerr << "gltf_fullpath is empty" << std::endl;
            return tinygltf::Model();
        }

        // Load the resource from file
        auto gltfModel = buildTinyModel(gltf_fullpath);

        // Cache the loaded resource
        resourceCache[id] = gltfModel;
        return gltfModel;
    }

private:
    // Private constructor for singleton
    GltfLoader() = default;

    // Resource cache
    std::map<uint32_t, tinygltf::Model> resourceCache;

    // TinyGLTF
    tinygltf::TinyGLTF tinygltfLoader = tinygltf::TinyGLTF();


    tinygltf::Model buildTinyModel(const std::string_view filename)
    {
        tinygltf::Model gltfModel;
        auto tStart = std::chrono::high_resolution_clock::now();
        std::string error;
        std::string warning;

        bool binary = false;
        size_t extpos = filename.rfind('.', filename.length());
        if (extpos != std::string::npos)
        {
            binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
        }

        bool fileLoaded = binary ? tinygltfLoader.LoadBinaryFromFile(&gltfModel,
            &error, &warning, std::string(filename).c_str()) :
            tinygltfLoader.LoadASCIIFromFile(&gltfModel, &error, &warning, std::string(filename).c_str());

        if (!fileLoaded)
        {
            std::cerr << "Could not load gltf file: " << error << std::endl;
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        std::cout << "GltfManager.buildTinyModel() Loaded file=" << filename << " in " << tDiff << "ms" << std::endl;
        return gltfModel;
    }
};

#endif