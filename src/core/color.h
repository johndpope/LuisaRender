//
// Created by Mike Smith on 2019/10/21.
//

#pragma once

#include "mathematics.h"

namespace luisa::color {

inline float RGB_to_Luminance(math::float3 rgb) {
    return math::dot(math::float3{0.2126729f, 0.7151522f, 0.0721750f}, rgb);
}

inline float XYZ_to_Luminance(math::float3 xyz) {
    return xyz.y;
}

inline float ACES_to_Luminance(math::float3 aces) {
    return math::dot(math::float3(0.3439664498f, 0.7281660966f, -0.0721325464f), aces);
}

inline float ACEScg_to_Luminance(math::float3 cg) {
    return math::dot(math::float3(0.2722287168f, 0.6740817658f, 0.0536895174f), cg.b);
}

inline math::float3 XYZ_to_RGB(math::float3 xyz) {
    return {math::dot(math::float3(3.2404542f, -1.5371385f, -0.4985314f), xyz),
            math::dot(math::float3(-0.9692660f, 1.8760108f, 0.0415560f), xyz),
            math::dot(math::float3(0.0556434f, -0.2040259f, 1.0572252f), xyz)};
}

inline math::float3 RGB_to_XYZ(math::float3 rgb) {
    return {math::dot(math::float3{0.4124564, 0.3575761f, 0.1804375f}, rgb),
            math::dot(math::float3{0.2126729, 0.7151522f, 0.0721750f}, rgb),
            math::dot(math::float3{0.0193339, 0.1191920f, 0.9503041f}, rgb)};
}

inline math::float3 ACES_to_XYZ(math::float3 aces) {
    return {math::dot(math::float3{0.9525523959f, 0.0000000000f, 0.0000936786f}, aces),
            math::dot(math::float3{0.3439664498f, 0.7281660966f, -0.0721325464f}, aces),
            math::dot(math::float3{0.0000000000f, 0.0000000000f, 1.0088251844f}, aces)};
}

inline math::float3 XYZ_to_ACES(math::float3 xyz) {
    return {math::dot(math::float3{1.0498110175f, 0.0000000000f, -0.0000974845f}, xyz),
            math::dot(math::float3{-0.4959030231f, 1.3733130458f, 0.0982400361f}, xyz),
            math::dot(math::float3{0.0000000000f, 0.0000000000f, 0.9912520182f}, xyz)};
}

inline math::float3 ACEScg_to_XYZ(math::float3 cg) {
    return {math::dot(math::float3{0.6624541811f, 0.1340042065f, 0.1561876870f}, cg),
            math::dot(math::float3{0.2722287168f, 0.6740817658f, 0.0536895174f}, cg),
            math::dot(math::float3{-0.0055746495f, 0.0040607335f, 1.0103391003f}, cg)};
}

inline math::float3 XYZ_to_ACEScg(math::float3 xyz) {
    return {math::dot(math::float3{1.6410233797f, -0.3248032942, -0.2364246952}, xyz),
            math::dot(math::float3{-0.6636628587f, 1.6153315917, 0.0167563477}, xyz),
            math::dot(math::float3{0.0117218943f, -0.0082844420, 0.9883948585}, xyz)};
}

}
