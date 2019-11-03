//
// Created by Mike Smith on 2019/10/31.
//

#pragma once

#ifndef DEVICE_COMPATIBLE

#include <cmath>
#include <cstdint>
#include <glm/glm.hpp>

namespace luisa::math {

using uint2 = glm::uvec2;

struct alignas(16) float3 : glm::vec3 {
    float padding [[maybe_unused]]{};
    using glm::vec3::vec;
    float3(glm::vec3 v) noexcept : glm::vec3{v} {}
};

using float2 = glm::vec2;
using float4 = glm::vec4;
using packed_float2 = glm::vec2;
using packed_float3 = glm::vec3;
using packed_float4 = glm::vec4;

using namespace glm;

template<typename T>
inline float dot(float3 lhs, T rhs) noexcept {
    return dot(packed_float3(lhs), rhs);
}

template<typename T>
inline float dot(T lhs, float3 rhs) noexcept {
    return dot(lhs, packed_float3(rhs));
}

inline float dot(float3 lhs, float3 rhs) noexcept {
    return dot(packed_float3(lhs), packed_float3(rhs));
}

}

#endif

namespace luisa::math {

inline namespace constants {

constexpr auto K_PI [[maybe_unused]] = 3.14159265358979323846264338327950288f;         // pi
constexpr auto K_PI_2 [[maybe_unused]] = 1.57079632679489661923132169163975144f;       // pi / 2
constexpr auto K_PI_4 [[maybe_unused]] = 0.785398163397448309615660845819875721f;      // pi / 4
constexpr auto K_1_PI [[maybe_unused]] = 0.318309886183790671537767526745028724f;      // 1 / pi
constexpr auto K_2_pi [[maybe_unused]] = 0.636619772367581343075535053490057448f;      // 2 / pi
constexpr auto K_2_SQRT_PI [[maybe_unused]] = 1.12837916709551257389615890312154517f;  // 2 / sqrt(pi)
constexpr auto K_SQRT_2 [[maybe_unused]] = 1.41421356237309504880168872420969808f;     // sqrt(2)
constexpr auto K_1_SQRT_2 [[maybe_unused]] = 0.707106781186547524400844362104849039f;  // 1 / sqrt(2)

}

}
