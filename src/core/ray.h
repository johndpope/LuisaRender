//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include "mathematics.h"

namespace luisa {

struct alignas(16) Ray {
    
    math::packed_float3 origin;
    float min_distance;
    
    math::packed_float3 direction;
    float max_distance;
    
    math::packed_float3 throughput;
    float pdf;
    
    math::packed_float3 radiance;
    uint32_t depth;
    
    math::float2 pixel;
    math::float2 padding{};
    
};

struct ShadowRay {
    math::packed_float3 origin;
    float min_distance;
    math::packed_float3 direction;
    float max_distance;
    math::packed_float3 light_radiance;
    float light_pdf;
};

struct GatherRay {
    math::float3 radiance;
    math::float2 pixel;
    math::float2 padding{};
};

}
