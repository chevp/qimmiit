/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <coregfx/ocean.hpp>

struct Textures
{
	oceantexture::TextureCubeMap environmentCube;
	oceantexture::Texture2D empty;
	oceantexture::Texture2D lutBrdf;
	oceantexture::TextureCubeMap irradianceCube;
	oceantexture::TextureCubeMap prefilteredCube;
};