//
// Created by Mike Smith on 2019/11/1.
//

#include "rgb_film.h"

namespace luisa {

void RGBFilm::convert_colorspace(KernelDispatcher &dispatch) {
    
    math::uint2 threadgroup_size{32, 32};
    auto threadgroups = (size() + threadgroup_size - 1u) / threadgroup_size;
    
    RGBFilmConvertColorspaceUniforms uniforms{size()};
    
    dispatch(*_convert_colorspace_kernel, threadgroups, threadgroup_size, [&](KernelArgumentEncoder &encoder) {
        encoder["uniforms"]->set_bytes(&uniforms, sizeof(RGBFilmConvertColorspaceUniforms));
        encoder["result"]->set_texture(*_texture);
    });
    
}

void RGBFilm::initialize(Device &device) {
    _convert_colorspace_kernel = device.create_kernel("rgb_film_convert_colorspace");
}

}
