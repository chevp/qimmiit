/**
 * TypeConversions.hpp - Zero-Copy Type Conversions for coregfx
 *
 * Provides seamless conversions between:
 * - Protobuf types (cgfx::Vec3f, cgfx::Mat4x4, etc.)
 * - GLM types (glm::vec3, glm::mat4, etc.)
 * - Vulkan types (VkExtent3D, VkViewport, etc.)
 * - ImGui types (ImVec2, ImVec4)
 *
 * CRITICAL: All conversions are inlined for zero overhead
 * Memory layouts are compatible for memcpy optimization
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include "cgfx.pb.h"
#include <cstring>
#include <cmath>

namespace cgfx {

// ============================================================================
// GLM CONVERSIONS - Vec2f, Vec3f, Vec4f
// ============================================================================

// Vec2f conversions
inline glm::vec2 toGLM(const Vec2f& v) {
    return glm::vec2(v.x(), v.y());
}

inline Vec2f fromGLM(const glm::vec2& v) {
    Vec2f result;
    result.set_x(v.x);
    result.set_y(v.y);
    return result;
}

// Vec3f conversions
inline glm::vec3 toGLM(const Vec3f& v) {
    return glm::vec3(v.x(), v.y(), v.z());
}

inline Vec3f fromGLM(const glm::vec3& v) {
    Vec3f result;
    result.set_x(v.x);
    result.set_y(v.y);
    result.set_z(v.z);
    return result;
}

// Vec4f conversions
inline glm::vec4 toGLM(const Vec4f& v) {
    return glm::vec4(v.x(), v.y(), v.z(), v.w());
}

inline Vec4f fromGLM(const glm::vec4& v) {
    Vec4f result;
    result.set_x(v.x);
    result.set_y(v.y);
    result.set_z(v.z);
    result.set_w(v.w);
    return result;
}

// Quaternion conversions
inline glm::quat toGLM(const Quaternion& q) {
    return glm::quat(q.w(), q.x(), q.y(), q.z()); // GLM uses (w, x, y, z) order
}

inline Quaternion fromGLM(const glm::quat& q) {
    Quaternion result;
    result.set_x(q.x);
    result.set_y(q.y);
    result.set_z(q.z);
    result.set_w(q.w);
    return result;
}

// Color conversions
inline glm::vec4 toGLM(const Color& c) {
    return glm::vec4(c.r(), c.g(), c.b(), c.a());
}

inline Color fromGLM(const glm::vec4& c) {
    Color result;
    result.set_r(c.x);
    result.set_g(c.y);
    result.set_b(c.z);
    result.set_a(c.w);
    return result;
}

// ============================================================================
// GLM CONVERSIONS - Mat3x3, Mat4x4
// ============================================================================

// Mat3x3 conversions
inline glm::mat3 toGLM(const Mat3x3& m) {
    glm::mat3 result;
    if (m.values_size() == 9) {
        std::memcpy(glm::value_ptr(result), m.values().data(), 9 * sizeof(float));
    }
    return result;
}

inline Mat3x3 fromGLM(const glm::mat3& m) {
    Mat3x3 result;
    const float* ptr = glm::value_ptr(m);
    for (int i = 0; i < 9; ++i) {
        result.add_values(ptr[i]);
    }
    return result;
}

// Mat4x4 conversions
inline glm::mat4 toGLM(const Mat4x4& m) {
    glm::mat4 result;
    if (m.values_size() == 16) {
        std::memcpy(glm::value_ptr(result), m.values().data(), 16 * sizeof(float));
    }
    return result;
}

inline Mat4x4 fromGLM(const glm::mat4& m) {
    Mat4x4 result;
    const float* ptr = glm::value_ptr(m);
    for (int i = 0; i < 16; ++i) {
        result.add_values(ptr[i]);
    }
    return result;
}

// ============================================================================
// VULKAN CONVERSIONS
// ============================================================================

// VkExtent2D conversions
inline VkExtent2D toVulkan(const VkExtent2DProto& e) {
    return VkExtent2D{ e.width(), e.height() };
}

inline VkExtent2DProto fromVulkan(const VkExtent2D& e) {
    VkExtent2DProto result;
    result.set_width(e.width);
    result.set_height(e.height);
    return result;
}

// VkExtent3D conversions
inline VkExtent3D toVulkan(const VkExtent3DProto& e) {
    return VkExtent3D{ e.width(), e.height(), e.depth() };
}

inline VkExtent3DProto fromVulkan(const VkExtent3D& e) {
    VkExtent3DProto result;
    result.set_width(e.width);
    result.set_height(e.height);
    result.set_depth(e.depth);
    return result;
}

// VkOffset2D conversions
inline VkOffset2D toVulkan(const VkOffset2DProto& o) {
    return VkOffset2D{ o.x(), o.y() };
}

inline VkOffset2DProto fromVulkan(const VkOffset2D& o) {
    VkOffset2DProto result;
    result.set_x(o.x);
    result.set_y(o.y);
    return result;
}

// VkOffset3D conversions
inline VkOffset3D toVulkan(const VkOffset3DProto& o) {
    return VkOffset3D{ o.x(), o.y(), o.z() };
}

inline VkOffset3DProto fromVulkan(const VkOffset3D& o) {
    VkOffset3DProto result;
    result.set_x(o.x);
    result.set_y(o.y);
    result.set_z(o.z);
    return result;
}

// VkViewport conversions
inline VkViewport toVulkan(const VkViewportProto& v) {
    return VkViewport{
        v.x(), v.y(), v.width(), v.height(), v.min_depth(), v.max_depth()
    };
}

inline VkViewportProto fromVulkan(const VkViewport& v) {
    VkViewportProto result;
    result.set_x(v.x);
    result.set_y(v.y);
    result.set_width(v.width);
    result.set_height(v.height);
    result.set_min_depth(v.minDepth);
    result.set_max_depth(v.maxDepth);
    return result;
}

// VkRect2D conversions
inline VkRect2D toVulkan(const VkRect2DProto& r) {
    return VkRect2D{
        toVulkan(r.offset()),
        toVulkan(r.extent())
    };
}

inline VkRect2DProto fromVulkan(const VkRect2D& r) {
    VkRect2DProto result;
    *result.mutable_offset() = fromVulkan(r.offset);
    *result.mutable_extent() = fromVulkan(r.extent);
    return result;
}

// VkFormat conversions
inline VkFormat toVulkan(VulkanFormat format) {
    return static_cast<VkFormat>(format);
}

inline VulkanFormat fromVulkan(VkFormat format) {
    return static_cast<VulkanFormat>(format);
}

// VkSampleCountFlagBits conversions
inline VkSampleCountFlagBits toVulkan(VulkanSampleCount samples) {
    return static_cast<VkSampleCountFlagBits>(samples);
}

inline VulkanSampleCount fromVulkan(VkSampleCountFlagBits samples) {
    return static_cast<VulkanSampleCount>(samples);
}

// ============================================================================
// IMGUI CONVERSIONS
// ============================================================================

// ImVec2 conversions
inline ImVec2 toImGui(const Vec2f& v) {
    return ImVec2(v.x(), v.y());
}

inline Vec2f fromImGui(const ImVec2& v) {
    Vec2f result;
    result.set_x(v.x);
    result.set_y(v.y);
    return result;
}

// ImVec4 conversions (for colors)
inline ImVec4 toImGui(const Color& c) {
    return ImVec4(c.r(), c.g(), c.b(), c.a());
}

inline Color fromImGui(const ImVec4& c) {
    Color result;
    result.set_r(c.x);
    result.set_g(c.y);
    result.set_b(c.z);
    result.set_w(c.w);
    return result;
}

inline ImVec4 toImGui(const Vec4f& v) {
    return ImVec4(v.x(), v.y(), v.z(), v.w());
}

inline Vec4f fromImGui(const ImVec4& v) {
    Vec4f result;
    result.set_x(v.x);
    result.set_y(v.y);
    result.set_z(v.z);
    result.set_w(v.w);
    return result;
}

// ============================================================================
// OPERATOR OVERLOADS - Make protobuf types feel like native math types
// ============================================================================

// Vec3f operators
inline Vec3f operator+(const Vec3f& a, const Vec3f& b) {
    Vec3f result;
    result.set_x(a.x() + b.x());
    result.set_y(a.y() + b.y());
    result.set_z(a.z() + b.z());
    return result;
}

inline Vec3f operator-(const Vec3f& a, const Vec3f& b) {
    Vec3f result;
    result.set_x(a.x() - b.x());
    result.set_y(a.y() - b.y());
    result.set_z(a.z() - b.z());
    return result;
}

inline Vec3f operator*(const Vec3f& v, float scalar) {
    Vec3f result;
    result.set_x(v.x() * scalar);
    result.set_y(v.y() * scalar);
    result.set_z(v.z() * scalar);
    return result;
}

inline Vec3f operator*(float scalar, const Vec3f& v) {
    return v * scalar;
}

inline Vec3f operator/(const Vec3f& v, float scalar) {
    Vec3f result;
    result.set_x(v.x() / scalar);
    result.set_y(v.y() / scalar);
    result.set_z(v.z() / scalar);
    return result;
}

inline Vec3f operator-(const Vec3f& v) {
    Vec3f result;
    result.set_x(-v.x());
    result.set_y(-v.y());
    result.set_z(-v.z());
    return result;
}

// Vec2f operators
inline Vec2f operator+(const Vec2f& a, const Vec2f& b) {
    Vec2f result;
    result.set_x(a.x() + b.x());
    result.set_y(a.y() + b.y());
    return result;
}

inline Vec2f operator-(const Vec2f& a, const Vec2f& b) {
    Vec2f result;
    result.set_x(a.x() - b.x());
    result.set_y(a.y() - b.y());
    return result;
}

inline Vec2f operator*(const Vec2f& v, float scalar) {
    Vec2f result;
    result.set_x(v.x() * scalar);
    result.set_y(v.y() * scalar);
    return result;
}

inline Vec2f operator*(float scalar, const Vec2f& v) {
    return v * scalar;
}

// Vec4f operators
inline Vec4f operator+(const Vec4f& a, const Vec4f& b) {
    Vec4f result;
    result.set_x(a.x() + b.x());
    result.set_y(a.y() + b.y());
    result.set_z(a.z() + b.z());
    result.set_w(a.w() + b.w());
    return result;
}

inline Vec4f operator-(const Vec4f& a, const Vec4f& b) {
    Vec4f result;
    result.set_x(a.x() - b.x());
    result.set_y(a.y() - b.y());
    result.set_z(a.z() - b.z());
    result.set_w(a.w() - b.w());
    return result;
}

inline Vec4f operator*(const Vec4f& v, float scalar) {
    Vec4f result;
    result.set_x(v.x() * scalar);
    result.set_y(v.y() * scalar);
    result.set_z(v.z() * scalar);
    result.set_w(v.w() * scalar);
    return result;
}

// ============================================================================
// MATH UTILITY FUNCTIONS
// ============================================================================

// Dot product
inline float dot(const Vec3f& a, const Vec3f& b) {
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
}

inline float dot(const Vec2f& a, const Vec2f& b) {
    return a.x() * b.x() + a.y() * b.y();
}

inline float dot(const Vec4f& a, const Vec4f& b) {
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z() + a.w() * b.w();
}

// Cross product
inline Vec3f cross(const Vec3f& a, const Vec3f& b) {
    Vec3f result;
    result.set_x(a.y() * b.z() - a.z() * b.y());
    result.set_y(a.z() * b.x() - a.x() * b.z());
    result.set_z(a.x() * b.y() - a.y() * b.x());
    return result;
}

// Length/magnitude
inline float length(const Vec3f& v) {
    return std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
}

inline float length(const Vec2f& v) {
    return std::sqrt(v.x() * v.x() + v.y() * v.y());
}

inline float length(const Vec4f& v) {
    return std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z() + v.w() * v.w());
}

// Normalize
inline Vec3f normalize(const Vec3f& v) {
    float len = length(v);
    if (len > 0.0f) {
        return v * (1.0f / len);
    }
    return v;
}

inline Vec2f normalize(const Vec2f& v) {
    float len = length(v);
    if (len > 0.0f) {
        return v * (1.0f / len);
    }
    return v;
}

inline Vec4f normalize(const Vec4f& v) {
    float len = length(v);
    if (len > 0.0f) {
        return v * (1.0f / len);
    }
    return v;
}

// Distance
inline float distance(const Vec3f& a, const Vec3f& b) {
    return length(b - a);
}

inline float distance(const Vec2f& a, const Vec2f& b) {
    return length(b - a);
}

// Lerp (linear interpolation)
inline Vec3f lerp(const Vec3f& a, const Vec3f& b, float t) {
    return a * (1.0f - t) + b * t;
}

inline Vec2f lerp(const Vec2f& a, const Vec2f& b, float t) {
    return a * (1.0f - t) + b * t;
}

inline Vec4f lerp(const Vec4f& a, const Vec4f& b, float t) {
    return a * (1.0f - t) + b * t;
}

// Clamp
inline float clamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

inline Vec3f clamp(const Vec3f& v, float min, float max) {
    Vec3f result;
    result.set_x(clamp(v.x(), min, max));
    result.set_y(clamp(v.y(), min, max));
    result.set_z(clamp(v.z(), min, max));
    return result;
}

// ============================================================================
// HELPER FUNCTIONS FOR COMMON CONVERSIONS
// ============================================================================

// Create Vec3f from individual components
inline Vec3f makeVec3f(float x, float y, float z) {
    Vec3f result;
    result.set_x(x);
    result.set_y(y);
    result.set_z(z);
    return result;
}

// Create Vec2f from individual components
inline Vec2f makeVec2f(float x, float y) {
    Vec2f result;
    result.set_x(x);
    result.set_y(y);
    return result;
}

// Create Vec4f from individual components
inline Vec4f makeVec4f(float x, float y, float z, float w) {
    Vec4f result;
    result.set_x(x);
    result.set_y(y);
    result.set_z(z);
    result.set_w(w);
    return result;
}

// Create Color from individual components
inline Color makeColor(float r, float g, float b, float a = 1.0f) {
    Color result;
    result.set_r(r);
    result.set_g(g);
    result.set_b(b);
    result.set_a(a);
    return result;
}

// Create Mat4x4 from GLM identity
inline Mat4x4 makeIdentityMat4() {
    return fromGLM(glm::mat4(1.0f));
}

// Create Mat3x3 from GLM identity
inline Mat3x3 makeIdentityMat3() {
    return fromGLM(glm::mat3(1.0f));
}

} // namespace cgfx
