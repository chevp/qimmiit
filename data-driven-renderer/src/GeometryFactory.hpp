/**
 * Geometry Factory
 *
 * Creates simple procedural geometry for testing shader instances
 * before full GLTF loading is implemented.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

namespace datadriven {

/**
 * Simple vertex structure for procedural geometry
 */
struct SimpleVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color;
};

/**
 * Geometry data for rendering
 */
struct GeometryData {
    std::vector<SimpleVertex> vertices;
    std::vector<uint32_t> indices;

    // Vulkan buffer handles (will be created by renderer)
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexMemory = VK_NULL_HANDLE;
};

/**
 * Factory for creating simple procedural geometry
 */
class GeometryFactory {
public:
    /**
     * Create a unit cube (1x1x1) centered at origin
     * @return Geometry data with vertices and indices
     */
    static GeometryData createCube() {
        GeometryData geo;

        // Define 8 vertices for a cube
        // Front face (z = 0.5)
        geo.vertices.push_back({{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}});

        // Back face (z = -0.5)
        geo.vertices.push_back({{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}});

        // Right face (x = 0.5)
        geo.vertices.push_back({{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 0.5f, 0.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f, 0.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {0.0f, 0.5f, 1.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.5f, 1.0f}});

        // Left face (x = -0.5)
        geo.vertices.push_back({{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {0.5f, 0.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 0.5f, 0.5f, 1.0f}});
        geo.vertices.push_back({{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {0.5f, 1.0f, 0.5f, 1.0f}});
        geo.vertices.push_back({{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {0.5f, 0.5f, 1.0f, 1.0f}});

        // Top face (y = 0.5)
        geo.vertices.push_back({{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.5f, 1.0f}});
        geo.vertices.push_back({{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 0.5f, 1.0f, 1.0f}});
        geo.vertices.push_back({{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}});

        // Bottom face (y = -0.5)
        geo.vertices.push_back({{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}, {0.2f, 0.2f, 0.2f, 1.0f}});
        geo.vertices.push_back({{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}, {0.3f, 0.3f, 0.3f, 1.0f}});
        geo.vertices.push_back({{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}});
        geo.vertices.push_back({{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}});

        // Define indices for triangles (2 triangles per face, 6 faces)
        uint32_t faceIndices[] = {
            0, 1, 2,  2, 3, 0,    // Front
            4, 5, 6,  6, 7, 4,    // Back
            8, 9, 10, 10, 11, 8,  // Right
            12, 13, 14, 14, 15, 12, // Left
            16, 17, 18, 18, 19, 16, // Top
            20, 21, 22, 22, 23, 20  // Bottom
        };

        geo.indices.assign(faceIndices, faceIndices + sizeof(faceIndices) / sizeof(uint32_t));

        return geo;
    }

    /**
     * Create a quad (2 triangles) on the XY plane
     * @return Geometry data with vertices and indices
     */
    static GeometryData createQuad() {
        GeometryData geo;

        // Define 4 vertices for a quad
        geo.vertices.push_back({{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
        geo.vertices.push_back({{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}});
        geo.vertices.push_back({{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}});
        geo.vertices.push_back({{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}});

        // Define indices for 2 triangles
        geo.indices = {0, 1, 2, 2, 3, 0};

        return geo;
    }
};

} // namespace datadriven
