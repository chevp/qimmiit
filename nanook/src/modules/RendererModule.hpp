/**
 * Cryo Engine - Renderer Module
 */

#pragma once

#include "../core/IModule.hpp"
#include <vulkan/vulkan.h>
#include <memory>

// Forward declare GLFW types
struct GLFWwindow;

namespace cryo {
namespace engine {

class RendererModule : public IModule {
public:
    RendererModule() = default;
    ~RendererModule() override = default;

    // IModule interface
    bool initialize(CentralAgent& agent) override;
    void update(float deltaTime) override;
    void shutdown() override;

    std::string getName() const override { return "RendererModule"; }
    ModuleType getType() const override { return ModuleType::RENDERER; }
    std::vector<std::string> getDependencies() const override;
    void handleMessage(const Message& msg) override;
    ModuleSpec getSpecification() const override;

private:
    bool initVulkan(GLFWwindow* window);
    void createInstance();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void recordCommandBuffer(uint32_t imageIndex);

    CentralAgent* agent_ = nullptr;
    GLFWwindow* window_ = nullptr;

    // Vulkan core objects
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;

    // Swapchain
    VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages_;
    std::vector<VkImageView> swapChainImageViews_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;
    VkFormat swapChainImageFormat_;
    VkExtent2D swapChainExtent_;

    // Rendering
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    // Synchronization
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    uint32_t currentFrame_ = 0;
    static const int MAX_FRAMES_IN_FLIGHT = 2;

    uint32_t graphicsFamily_ = 0;
    uint32_t presentFamily_ = 0;
};

} // namespace engine
} // namespace cryo