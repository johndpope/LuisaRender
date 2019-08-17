#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

kernel void split_channel(
    texture2d<float, access::read> rgb [[texture(0)]],
    device float *r [[buffer(0)]],
    device float *g [[buffer(1)]],
    device float *b [[buffer(2)]],
    uint2 coord [[thread_position_in_grid]],
    uint2 size [[threads_per_grid]]) {
    auto pixel = rgb.read(coord);
    auto index = coord.x + coord.y * size.x;
    r[index] = pixel.r;
    g[index] = pixel.g;
    b[index] = pixel.b;
}