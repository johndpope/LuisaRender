//
// Created by Mike Smith on 2019/11/1.
//

#include "mitchell_netravali_filter.h"

namespace luisa {

void MitchellNetravaliFilter::apply(KernelDispatcher &dispatch, Buffer &gather_ray_buffer, Texture &result_texture, math::uint2 frame_size, uint32_t frame_index) {
    
    math::uint2 threadgroup_size{32, 32};
    auto threadgroups = (frame_size + threadgroup_size - 1u) / threadgroup_size;
    
    MitchellNetravaliFilterApplyUniforms uniforms{frame_size, frame_index, _radius, _b, _c};
    
    dispatch(*_apply_kernel, threadgroups, threadgroup_size, [&](KernelArgumentEncoder &encoder) {
        encoder["uniforms"]->set_bytes(&uniforms, sizeof(MitchellNetravaliFilterApplyUniforms));
        encoder["rays"]->set_buffer(gather_ray_buffer);
        encoder["result"]->set_texture(result_texture);
    });
}

void MitchellNetravaliFilter::initialize(Device &device) {
    _apply_kernel = device.create_kernel("mitchell_natravali_filter_apply");
}

}
