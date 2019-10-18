#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Material {
    texture2d<float> diffuse;
    texture2d<float> specular;
};

struct Light {
    float3 position;
    float3 emission;
    float3 area;
};

kernel void background(
    texture2d<float, access::write> image [[texture(0)]],
    constant Material *materials [[buffer(0)]],
    constant Light *lights [[buffer(1)]],
    uint2 coordinates [[thread_position_in_grid]],
    uint2 size [[threads_per_grid]]) {
    
    float2 uv = float2(coordinates) / float2(size - 1) * 2.0f - 1.0f;
    image.write(float4(uv * uv, 0.5f - dot(uv, uv) * 0.5f, 1.0f), coordinates);
}
