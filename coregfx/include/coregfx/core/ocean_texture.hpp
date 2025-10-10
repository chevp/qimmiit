/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef OCEAN_TEXTURE_HPP_
#define OCEAN_TEXTURE_HPP_

#include <iostream>
#include <stdexcept>
#include <stb_image.h> // Bibliothek für das Laden von Bildern

#ifndef OCEAN_DEPENDENCIES_HPP_
#include "ocean_dependencies.hpp"
#endif

#ifndef OCEAN_MACROS_HPP_
#include "ocean_macros.hpp"
#endif

#ifndef OCEAN_CORE_HPP_
#include "ocean_core.hpp"
#endif

namespace oceantexture
{
	class Texture
	{
	public:
		oceancore::VulkanDevice *device;
		VkImage image = VK_NULL_HANDLE;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;

		void updateDescriptor();
		void destroy();
	};

	class Texture2D : public Texture
	{
	public:
		// unused
		void loadFromFile(
			std::string_view filename,
			VkFormat format,
			oceancore::VulkanDevice *device,
			VkQueue copyQueue,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// with chatgpt, not tested, unused
		void loadFromFileOptimized(
			std::string_view filename,
			VkFormat format,
			oceancore::VulkanDevice* device,
			VkQueue copyQueue,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		void loadFromBuffer(
			void *buffer,
			VkDeviceSize bufferSize,
			VkFormat format,
			uint32_t width,
			uint32_t height,
			oceancore::VulkanDevice *device,
			VkQueue copyQueue,
			VkFilter filter = VK_FILTER_LINEAR,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};

	class TextureCubeMap : public Texture
	{
	public:
		void loadFromFile(
			std::string_view filename,
			VkFormat format,
			oceancore::VulkanDevice *device,
			VkQueue copyQueue,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
}

#endif