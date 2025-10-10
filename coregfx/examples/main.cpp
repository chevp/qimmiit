/**
 * Minimal example demonstrating coregfx library usage
 *
 * This example shows:
 * 1. Thread pool usage for concurrent tasks
 * 2. Resource manager for file loading/caching
 * 3. GLTF model loading
 * 4. XML Elyrion scene file parsing
 *
 * Copyright (C) by Patrice Chevillat
 */

#include <iostream>
#include <string>
#include <future>

// Core utilities (Vulkan/gRPC independent)
#include <coregfx/util/thread_pool.hpp>
#include <coregfx/rsc/rsc_manager.hpp>
#include <coregfx/gltf/gltf_loader.hpp>
#include <coregfx/rsc/xml_elyrion.hpp>

// Protobuf wrapper
#include <coregfx/protobuf_wrapper.hpp>

void demonstrateThreadPool() {
    std::cout << "\n=== Thread Pool Example ===\n";

    // Create thread pool with 4 worker threads
    coregfx::ThreadPool pool(4);

    // Enqueue tasks
    std::vector<std::future<int>> results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "Task " << i << " running on thread "
                          << std::this_thread::get_id() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return i * i;
            })
        );
    }

    // Collect results
    for(auto& result : results) {
        std::cout << "Result: " << result.get() << std::endl;
    }
}

void demonstrateResourceManager() {
    std::cout << "\n=== Resource Manager Example ===\n";

    // Get singleton instance
    auto& resourceManager = ResourceManager::getInstance();

    // Create a test file
    std::string testPath = "test_resource.txt";
    {
        std::ofstream testFile(testPath);
        testFile << "This is a test resource file.\n";
        testFile << "Resource managers cache file contents for fast access.\n";
    }

    // Load resource (first time - from disk)
    std::string content1 = resourceManager.load(testPath);
    std::cout << "Loaded from disk:\n" << content1 << std::endl;

    // Load again (cached - fast)
    std::string content2 = resourceManager.load(testPath);
    std::cout << "Loaded from cache (same content): "
              << (content1 == content2 ? "YES" : "NO") << std::endl;

    // Clean up
    std::remove(testPath.c_str());
}

void demonstrateGltfLoader() {
    std::cout << "\n=== GLTF Loader Example ===\n";

    // Get singleton instance
    auto& gltfLoader = GltfLoader::getInstance();

    // Note: This would load a real GLTF file if available
    // For demonstration, we show the API usage
    std::string gltfPath = "path/to/model.gltf";

    std::cout << "GLTF Loader API example:\n";
    std::cout << "  auto& loader = GltfLoader::getInstance();\n";
    std::cout << "  tinygltf::Model model = loader.loadGltfRsc(modelId, \"" << gltfPath << "\");\n";
    std::cout << "  // Model is now cached and ready to use\n";
    std::cout << "\nNote: Actual loading requires valid GLTF file path\n";
}

void demonstrateElyrionXmlParser() {
    std::cout << "\n=== Elyrion XML Parser Example ===\n";

    // Create a minimal Elyrion XML file
    std::string xmlPath = "example_scene.elyrion.xml";
    {
        std::ofstream xmlFile(xmlPath);
        xmlFile << R"(<?xml version="1.0" encoding="UTF-8"?>
<elyrion id="example" version="1.0.0" author="coregfx-example">
    <name>Example Scene</name>
    <description>Minimal example scene for coregfx demonstration</description>
    <entryScene>main</entryScene>

    <assets>
        <asset id="101" type="gltf" version="2.0" src="models/cube.gltf" />
        <asset id="102" type="gltf" version="2.0" src="models/sphere.gltf" />
    </assets>

    <scenes>
        <scene id="main" name="Main Scene">
            <entities>
                <entity id="cube-entity" type="mesh" asset="101" />
                <entity id="sphere-entity" type="mesh" asset="102" />
            </entities>
        </scene>
    </scenes>
</elyrion>
)";
    }

    // Parse the XML file into Elyrion protobuf model
    nyx::Elyrion elyrionModel;
    bool success = xmlelyrion::ElyrionLoader::loadXmlFromFile(xmlPath, elyrionModel);

    if (success) {
        std::cout << "Successfully parsed Elyrion XML!\n";
        std::cout << "  ID: " << elyrionModel.id() << "\n";
        std::cout << "  Name: " << elyrionModel.name() << "\n";
        std::cout << "  Author: " << elyrionModel.author() << "\n";
        std::cout << "  Version: " << elyrionModel.version() << "\n";
        std::cout << "  Entry Scene: " << elyrionModel.entryscene() << "\n";
        std::cout << "  Assets: " << elyrionModel.assets_size() << "\n";
        std::cout << "  Scenes: " << elyrionModel.scenes_size() << "\n";

        // List assets
        for (int i = 0; i < elyrionModel.assets_size(); ++i) {
            const auto& asset = elyrionModel.assets(i);
            std::cout << "    Asset[" << i << "]: id=" << asset.id()
                      << ", type=" << asset.type()
                      << ", src=" << asset.src() << "\n";
        }

        // List scenes and entities
        for (int i = 0; i < elyrionModel.scenes_size(); ++i) {
            const auto& scene = elyrionModel.scenes(i);
            std::cout << "    Scene[" << i << "]: id=" << scene.id()
                      << ", name=" << scene.name()
                      << ", entities=" << scene.entities_size() << "\n";

            for (int j = 0; j < scene.entities_size(); ++j) {
                const auto& entity = scene.entities(j);
                std::cout << "      Entity[" << j << "]: id=" << entity.id()
                          << ", type=" << entity.type()
                          << ", asset=" << entity.asset() << "\n";
            }
        }
    } else {
        std::cout << "Failed to parse Elyrion XML\n";
    }

    // Clean up
    std::remove(xmlPath.c_str());
}

int main(int argc, char** argv) {
    std::cout << "=================================================\n";
    std::cout << "       coregfx Library Example Usage\n";
    std::cout << "=================================================\n";

    try {
        // Demonstrate independent utilities
        demonstrateThreadPool();
        demonstrateResourceManager();
        demonstrateGltfLoader();
        demonstrateElyrionXmlParser();

        std::cout << "\n=================================================\n";
        std::cout << "All examples completed successfully!\n";
        std::cout << "=================================================\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
