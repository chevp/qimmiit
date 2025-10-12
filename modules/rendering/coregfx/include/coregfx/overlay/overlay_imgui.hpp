/*
 * Vulkan Example - Physical based rendering a glTF 2.0 model with image based lighting
 *
 * Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef OVERLAY_IMGUI_HPP_
#define OVERLAY_IMGUI_HPP_

#include <vector>
#include <array>
#include <map>
#include <string>

#ifndef OCEAN_HPP
#include <coregfx/ocean.hpp>
#endif

#include <coregfx/core/ocean_texture.hpp>
#include <imgui/imgui.h>

namespace overlayimgui
{
    class UI
    {
    public:
        UI(oceancore::VulkanDevice *vulkanDevice, VkRenderPass renderPass, VkQueue queue,
           VkPipelineCache pipelineCache, VkSampleCountFlagBits multiSampleCount);
        ~UI();

        void init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device,
                  uint32_t queueFamily, VkQueue queue, VkPipelineCache pipelineCache,
                  VkRenderPass renderPass, uint32_t subpass = 0);

        void cleanup();
        void draw(VkCommandBuffer commandBuffer);
        void resize(uint32_t width, uint32_t height);

        // Add other public interface methods

        // UI helper methods
        bool header(const char* caption);
        bool slider(const char* caption, float* value, float min, float max);
        bool combo(const char* caption, int32_t* itemindex, std::vector<std::string> items);
        bool combo(const char* caption, std::string& selectedkey, std::map<std::string, std::string> items);
        bool button(const char* caption);
        bool checkbox(const char* caption, bool* value);
        void text(const char* format, ...);

        // Studio mode menu bar functionality
        void beginMainMenuBar();
        void endMainMenuBar();
        bool beginMenu(const char* label);
        void endMenu();
        bool menuItem(const char* label, const char* shortcut = nullptr, bool selected = false);

        struct PushConstBlock {
            glm::vec2 scale;
            glm::vec2 translate;
        } pushConstBlock;

        oceancore::Buffer vertexBuffer;
        oceancore::Buffer indexBuffer;

    private:
        VkDevice device = VK_NULL_HANDLE;
        oceantexture::Texture2D fontTexture;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
}

#endif