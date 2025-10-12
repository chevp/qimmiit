/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef OCEAN_CORE_HPP_
#define OCEAN_CORE_HPP_

#ifndef OCEAN_MACROS_HPP_
#include "ocean_macros.hpp"
#endif

#ifndef OCEAN_INITIALIZER_HPP_
#include "ocean_initializer.hpp"
#endif

#include <tinygltf/tiny_gltf.h>

#include <string>
#include <string_view>

namespace oceancore
{
	struct VulkanDevice
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceFeatures enabledFeatures;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkQueue graphicsQueue;   // Grafikkarten-Queue (chatgpt)

		struct
		{
			uint32_t graphics;
			uint32_t compute;
		} queueFamilyIndices;

		// operator VkDevice();
		VulkanDevice(VkPhysicalDevice physicalDevice);
		~VulkanDevice();
		uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
		uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags);
		VkResult createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, 
			VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
		VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, std::string msg, void* data = nullptr);
		VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, 
			VkBuffer* buffer, VkDeviceMemory* memory, void* data);
		VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
		void beginCommandBuffer(VkCommandBuffer commandBuffer);
		void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void logMemoryInfo();
	private:
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	};

	class Camera
	{
	private:
		float fov;
		float znear, zfar;
		void updateViewMatrix();

	public:
		enum CameraType
		{
			lookat,
			firstperson
		};
		CameraType type = CameraType::lookat;

		glm::vec3 rotation = glm::vec3();
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 10.0f);

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;

		bool updated = false;

		struct
		{
			glm::mat4 perspective;
			glm::mat4 view;
		} matrices;

		struct
		{
			bool left = false;
			bool right = false;
			bool up = false;
			bool down = false;
		} keys;

		bool moving();
		float getNearClip();
		float getFarClip();
		std::string debugString();
		void setPerspective(float fov, float aspect, float znear, float zfar);
		void updateAspectRatio(float aspect);
		void setPosition(glm::vec3 position);
		void setRotation(glm::vec3 rotation);
		void rotate(glm::vec3 delta);
		void setTranslation(glm::vec3 translation);
		void translate(glm::vec3 delta);
		void update(float deltaTime);
		bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime);
	};

	// https://andi-siess.de/rgb-to-color-temperature/
	// 2400k = (255, 157, 63)	= (1, 0.62, 0.25)
	// 5800k = (255, 240, 233)	= (1, 0.94, 0.91)
	struct LightSource
	{
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
	};

	// Vulkan buffer object
	struct Buffer
	{
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		int32_t count = 0;
		void *mapped = nullptr;
		void create(VulkanDevice *device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
					VkDeviceSize size, bool map = true);
		void destroy();
		void map();
		void unmap();
		void flush(VkDeviceSize size = VK_WHOLE_SIZE);
	};

	struct Settings
	{
		bool validation = false;
		bool fullscreen = false;
		bool vsync = false;
		bool multiSampling = true;
		bool overlay = false;
		uint32_t windowHeight = 0;
		uint32_t windowWidth = 0;
		uint32_t width = 0;
		uint32_t height = 0;
		std::string title = "Vulkan Application";
		bool useDiscreteGpu = false;
		bool enableStudioMode = false;
		bool headless = false;
	};

	// Forward declaration for Ocean type alias
	// Ocean is an alias for VulkanExampleBase, defined in ocean_base.hpp
	// This provides a simpler name for applications to use
}

#endif