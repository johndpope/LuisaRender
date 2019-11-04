//
// Created by Mike Smith on 2019/11/1.
//

#include "compatibility.h"

#include <core/ray.h>
#include <filters/mitchell_netravali_filter.h>

using namespace luisa;
using namespace math;
using namespace metal;

inline float Mitchell1D(float b, float c, float dx) {
    auto x = min(abs(2 * dx), 2.0f);
    auto xx = x * x;
    return (1.0f / 6.0f) *
           (x > 1 ?
            (-(b + 6.0f * c) * xx + 6.0f * (b + 5.0f * c) * x - 12.0f * (b + 4.0f * c)) * x + 8.0f * (b + 3.0f * c) :
            (-6.0f * (1.5f * b + c - 2.0f) * xx + 6.0f * (2.0f * b + c - 3.0f) * x) * x - 2.0f * (b - 3.0f));
}

kernel void mitchell_natravali_filter_apply(
    constant MitchellNetravaliFilterApplyUniforms &uniforms [[buffer(0)]],
    device const GatherRay *rays [[buffer(1)]],
    texture2d<float, access::read_write> result [[texture(0)]],
    uint2 tid [[thread_position_in_grid]]) {
    
    if (tid.x < uniforms.frame_size.x && tid.y < uniforms.frame_size.y) {
        
        auto pixel_radius = static_cast<uint32_t>(ceil(uniforms.radius - 0.5f - 1e-4f));
        auto inv_filter_radius = 1.0f / uniforms.radius;
        
        auto min_x = max(tid.x, pixel_radius) - pixel_radius;
        auto min_y = max(tid.y, pixel_radius) - pixel_radius;
        auto max_x = min(tid.x + pixel_radius, uniforms.frame_size.x - 1u);
        auto max_y = min(tid.y + pixel_radius, uniforms.frame_size.y - 1u);
        
        float3 radiance_sum{};
        auto weight_sum = 0.0f;
        auto center = float2(tid) + 0.5f;
        for (auto y = min_y; y <= max_y; y++) {
            for (auto x = min_x; x <= max_x; x++) {
                auto index = y * uniforms.frame_size.x + x;
                auto radiance = rays[index].radiance;
                auto pixel = rays[index].pixel;
                auto dx = (center.x - pixel.x) * inv_filter_radius;
                auto dy = (center.y - pixel.y) * inv_filter_radius;
                auto weight = Mitchell1D(uniforms.b, uniforms.c, dx) * Mitchell1D(uniforms.b, uniforms.c, dy);
                radiance_sum += weight * radiance;
                weight_sum += weight;
            }
        }
        result.write(mix(result.read(tid), float4(radiance_sum, weight_sum), 1.0f / (uniforms.frame_index + 1.0f)), tid);
    }
}
