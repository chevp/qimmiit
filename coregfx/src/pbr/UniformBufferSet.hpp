/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <coregfx/ocean.hpp>

struct UniformBufferSet
{
    oceancore::Buffer scene;
    oceancore::Buffer skybox;
    oceancore::Buffer params;
};