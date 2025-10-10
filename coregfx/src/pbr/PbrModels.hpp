/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <coregfx/ocean.hpp>
#include <vector>
#include <vulkan/vulkan.h>

struct PbrModels
{
	oceangltf::Model scene;
	oceangltf::Model skybox;

	oceangltf::Model *gltf2 = nullptr;

	std::vector<oceangltf::Model> gltfs;

	PbrModels() {}

	void destroy(VkDevice device)
	{
		scene.destroy(device);

		if (gltf2 != nullptr)
		{
			gltf2->destroy(device);
			delete gltf2;
		}

		skybox.destroy(device);
	}
};