/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef VULKAN_CONTEXT_HPP_
#define VULKAN_CONTEXT_HPP_

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <iostream>
#include <vector>

class VulkanContext {
public:
    // Singleton access method
    static VulkanContext& getInstance() {
        static VulkanContext instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copies
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // Initialize Vulkan context (called once)
    void initialize() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    // Public properties for access
    VkDevice getDevice() const { return device; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    VkCommandPool getCommandPool() const { return commandPool; }
    VkSurfaceKHR getSurface() const { return surface; }

    // Cleanup Vulkan resources
    void cleanup() {

        if (debugMessenger != VK_NULL_HANDLE) {
            destroyDebugMessenger(nullptr);
        }

        if (device != VK_NULL_HANDLE && commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, commandPool, nullptr);
        }

        if (device != VK_NULL_HANDLE) {
            vkDestroyDevice(device, nullptr);
        }

        if (instance != VK_NULL_HANDLE) {
            vkDestroyInstance(instance, nullptr);
        }

        if (instance != VK_NULL_HANDLE && surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }
    }
private:
    VulkanContext() = default;  // Private constructor for Singleton

    // Vulkan Properties
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // Layers and extensions
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    bool enableValidationLayers = true;

    // Helper function to check validation layer support
    bool checkValidationLayerSupport();

    // Vulkan setup functions
    void createInstance();

    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device) {
        // Evaluate if the device supports required features (graphics, presentation, etc.)
        return true;
    }

    void createLogicalDevice();
    void createCommandPool();

    // Function to destroy Debug Messenger
    void destroyDebugMessenger(const VkAllocationCallbacks* pAllocator);
};

#endif