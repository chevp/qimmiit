/*
ImStudio Stubs - Minimal Implementation for Build Compatibility
*/

// Forward declarations to avoid including conflicting headers
namespace cgfx {
    class Layout;
}

#include <string>

namespace ImStudio {
    void applyImGuiStyle(const cgfx::Layout& layout, const std::string& activeItemId) {
        // Stub implementation
    }

    void renderImGuiFromProto(const cgfx::Layout& layout, const std::string& activeItemId) {
        // Stub implementation - minimal functionality to satisfy linker
        // This would normally render ImGui UI elements based on the protobuf layout
    }
}