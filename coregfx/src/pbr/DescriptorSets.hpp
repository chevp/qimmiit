/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

struct DescriptorSets
{
	VkDescriptorSet scene;
	VkDescriptorSet skybox;

	VkDescriptorSet gltf2;

	std::vector<VkDescriptorSet> gltfs;
};