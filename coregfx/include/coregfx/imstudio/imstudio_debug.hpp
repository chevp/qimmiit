/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef IMSTUDIO_DEBUG
#define IMSTUDIO_DEBUG

#include <coregfx/protobuf_wrapper.hpp>
#include "cgfx.pb.h"
#include <imgui/imgui.h>

namespace ImStudio
{
    // Function to apply styles if provided in the Proto message
    void applyImGuiStyle(const cgfx::Layout& layout, const std::string& activeItemId);
    void renderImGuiFromProto(const cgfx::Layout& layout, const std::string& activeItemId);

    // Create a default layout configuration
    inline cgfx::Layout createDefaultLayout2() {
        cgfx::Layout layout;
        // TODO: Implement proper default layout configuration
        // This is a stub implementation
        return layout;
    }

    // Note: LayoutItem type may not exist in current protobuf definitions
    // void renderItemsRecursive(const cgfx::LayoutItem& item, const std::string& activeItemId);
    // void renderItem(const cgfx::LayoutItem& item, const std::string& activeItemId);
}

#endif