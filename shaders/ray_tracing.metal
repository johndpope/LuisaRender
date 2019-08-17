#ifdef __METAL_VERSION__
# include <metal_stdlib>
# include <simd/simd.h>
#else

# define constant const
# define kernel
# define device

# include <simd/simd.h>

namespace metal {

using namespace simd;

enum struct access {
    read, write, read_write
};

template<typename T, access acc = access::read>
struct texture2d {};

}

#endif

#include "../ray.h"
#include "../intersection.h"
#include "../camera.h"

using namespace metal;

constant unsigned int primes[] = {
    2, 3, 5, 7,
    11, 13, 17, 19,
    23, 29, 31, 37,
    41, 43, 47, 53,
};

float halton(unsigned int i, unsigned int d) {
    unsigned int b = primes[d];
    float f = 1.0f;
    float invB = 1.0f / b;
    float r = 0;
    while (i > 0) {
        f = f * invB;
        r = r + f * (i % b);
        i = i / b;
    }
    return r;
}

inline float2 uniform_sample_disk(simd::float2 random) {
    
    constexpr auto PI = 3.14159265358979323846264338327950288f;
    
    random = random * 2.0f - 1.0f;
    if (random.x != 0.0f && random.y != 0.0f) {
        float r, theta;
        if (abs(random.x) > abs(random.y)) {
            r = random.x;
            theta = (PI * 0.25f) * (random.y / random.x);
        } else {
            r = random.y;
            theta = (PI * 0.5f) - (PI * 0.25f) * (random.x / random.y);
        }
        random.x = r * cos(theta);
        random.y = r * sin(theta);
    }
    return random;
}

inline float3 view_transform_position(constant Camera &camera, simd::float3 p) {
    return p.x * camera._left + p.y * camera._up + p.z * camera._front + camera._position;
}

inline float3 view_transform_normal(constant Camera &camera, simd::float3 n) {
    return normalize(n.x * camera._left + n.y * camera._up + n.z * camera._front);
}

inline Ray sample_ray(constant Camera &camera, simd::float2 xy, simd::float2 random = {}) {
    auto p_near_plane = float3(xy * camera._frame_width, camera._near_plane);
    if (camera._lens_radius == 0.0f) {
        return {camera._position, 0.0f, normalize(view_transform_position(camera, p_near_plane) - camera._position), INFINITY};
    }
    auto p_lens = float3(camera._lens_radius * uniform_sample_disk(random), 0.0f);
    auto p_focal_plane = p_near_plane * camera._focal_distance / camera._near_plane;
    return {view_transform_position(camera, p_lens), camera._near_plane, view_transform_normal(camera, p_focal_plane - p_lens), INFINITY};
}

kernel void generate_ray(device Ray *rays [[buffer(0)]],
                         constant Camera &camera [[buffer(1)]],
                         constant uint32_t &random_offset [[buffer(2)]],
                         texture2d<uint32_t, access::read> random [[texture(0)]],
                         uint2 coords [[thread_position_in_grid]],
                         uint2 size [[threads_per_grid]]) {
    uint rayIndex = coords.x + coords.y * size.x;
    auto seed = random.read(coords).r + random_offset;
    auto xy = float2(coords) + float2(halton(seed, 0), halton(seed, 1));
    rays[rayIndex] = sample_ray(camera, (0.5f * float2(size) - xy) / size.x, {halton(seed, 2), halton(seed, 3)});
}

kernel void handle_intersection(texture2d<float, access::write> image [[texture(0)]],
                                device const Intersection *intersections [[buffer(0)]],
                                uint2 coordinates [[thread_position_in_grid]],
                                uint2 size [[threads_per_grid]]) {
    uint rayIndex = coordinates.x + coordinates.y * size.x;
    device const Intersection &i = intersections[rayIndex];
    if (i.distance > 0.0f) {
        float w = 1.0 - i.coordinates.x - i.coordinates.y;
        image.write(float4(i.coordinates, w, 1.0), coordinates);
    }
}

kernel void accumulate(texture2d<float, access::read> frame [[texture(0)]],
                       texture2d<float, access::read_write> accum [[texture(1)]],
                       constant uint32_t &frame_count [[buffer(0)]],
                       uint2 coords [[thread_position_in_grid]]) {
    auto color = frame.read(coords).xyz;
    auto total = accum.read(coords).xyz;
    accum.write(float4((total * (frame_count - 1) + color) / frame_count, 1.0f), coords);
}