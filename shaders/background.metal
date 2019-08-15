#include <metal_stdlib>
using namespace metal;

kernel void background(texture2d<float, access::write> image [[texture(0)]],
                          uint2 coordinates [[thread_position_in_grid]],
                          uint2 size [[threads_per_grid]]) {
    float2 uv = float2(coordinates) / float2(size - 1) * 2.0f - 1.0f;
    image.write(float4(uv * uv, dot(uv, uv) * 0.5f + 0.5f, 1.0f), coordinates);
}
