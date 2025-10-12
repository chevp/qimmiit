#pragma once

#ifndef VULKAN_RENDERER_HPP_
#define VULKAN_ENDERER_HPP_

class VulkanRenderer {
public:
    virtual ~VulkanRenderer() = default;

    // Pure virtual function to be implemented by derived classes
    virtual void render() = 0;

    // Function to cleanup resources (if needed)
    virtual void cleanup() = 0;
};

#endif