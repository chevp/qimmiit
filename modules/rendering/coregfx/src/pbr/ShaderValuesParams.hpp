/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

struct ShaderValuesParams
{
	glm::vec4 lightDir;
	float exposure = 4.5f;
	float gamma = 2.2f;
	float prefilteredCubeMipLevels;
	float scaleIBLAmbient = 0.75f;
	float debugViewInputs = 0;
	float debugViewEquation = 0;
};