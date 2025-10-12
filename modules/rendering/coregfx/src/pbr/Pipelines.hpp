/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

struct Pipelines
{
	VkPipeline skybox;
	VkPipeline pbr;
	VkPipeline pbrDoubleSided;
	VkPipeline pbrAlphaBlend;
};