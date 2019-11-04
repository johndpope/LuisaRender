#include "compatibility.h"

#include <core/ray.h>
#include <cameras/pinhole_camera.h>

using namespace luisa;
using namespace math;
using namespace metal;

kernel void pinhole_camera_generate_rays(
    constant PinholeCameraGenerateRaysUniforms &uniforms [[buffer(0)]],
    device Ray *rays [[buffer(1)]],
    texture2d<float, access::read> random [[texture(0)]],
    uint2 tid [[thread_position_in_grid]]) {
    
    if (tid.x < uniforms.frame_size.x && tid.y < uniforms.frame_size.y) {
        
        auto r = random.read(tid);
        auto pixel = float2(tid) + float2{r.x, r.y};
        auto size = float2(uniforms.frame_size);
        
        auto sensor = (0.5f - pixel / size) * uniforms.sensor_size;
        
        Ray ray{};
        ray.origin = uniforms.position;
        ray.direction = normalize(sensor.x * uniforms.left + sensor.y * uniforms.up + uniforms.near_plane * uniforms.front);
        ray.min_distance = 0.0f;
        ray.max_distance = INFINITY;
        ray.throughput = {1.0f, 1.0f, 1.0f};
        ray.radiance = {0.0f, 0.0f, 0.0f};
        ray.depth = 0;
        ray.pixel = pixel;
        
        rays[tid.y * uniforms.frame_size.x + tid.x] = ray;
    }
}
