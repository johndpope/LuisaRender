//
// Created by Mike Smith on 2019/10/21.
//

#include "compatibility.h"

#include <core/color.h>
#include <films/rgb_film.h>

using namespace luisa;
using namespace math;
using namespace metal;

kernel void rgb_film_convert_colorspace(
    constant RGBFilmConvertColorspaceUniforms &uniforms [[buffer(0)]],
    texture2d<float, access::read_write> result [[texture(0)]],
    uint2 tid [[thread_position_in_grid]]) {
    
    if (tid.x < uniforms.frame_size.x && tid.y < uniforms.frame_size.y) {
        auto f = result.read(tid);
        if (f.a == 0.0f) { f.a = 1e-3f; }
        result.write(float4(color::XYZ_to_RGB(color::ACEScg_to_XYZ(float3(f) / f.a)), 1.0f), tid);
    }
    
}
