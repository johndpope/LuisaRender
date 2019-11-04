//
// Created by Mike Smith on 2019/10/31.
//

#include "halton_sampler.h"

namespace luisa {

void HaltonSampler::initialize(Device &device, const CoreTypeInitializerParameterSet &param_set [[maybe_unused]]) {
    Sampler::initialize(device, param_set);
    _prepare_for_frame_kernel = device.create_kernel("halton_sampler_prepare_for_frame");
    _generate_samples_kernel = device.create_kernel("halton_sampler_generate_samples");
    _device = &device;
}

void HaltonSampler::generate_samples(KernelDispatcher &dispatch, Texture &random_texture, uint dimensions) {
    
    math::uint2 threadgroup_size{32, 32};
    auto threadgroups = (_frame_size + threadgroup_size - 1u) / threadgroup_size;
    
    HaltonSamplerGenerateSamplesUniforms uniforms{};
    uniforms.frame_size = _frame_size;
    uniforms.dimensions = dimensions;
    
    dispatch(*_generate_samples_kernel, threadgroups, threadgroup_size, [&](KernelArgumentEncoder &encoder) {
        encoder["uniforms"]->set_bytes(&uniforms, sizeof(HaltonSamplerGenerateSamplesUniforms));
        encoder["states"]->set_buffer(*_state_buffer);
        encoder["random"]->set_texture(random_texture);
    });
    
    _current_dimension += dimensions;
}

void HaltonSampler::prepare_for_frame(KernelDispatcher &dispatch, math::uint2 frame_size, uint frame_index, uint total_dimensions) {
    
    _current_dimension = 0u;
    if (_frame_size != frame_size) {
        _state_buffer = _device->create_buffer(sizeof(HaltonSamplerState) * frame_size.x * frame_size.y, BufferStorageTag::DEVICE_PRIVATE);
        _frame_size = frame_size;
    }
    
    math::uint2 threadgroup_size{32, 32};
    auto threadgroups = (frame_size + threadgroup_size - 1u) / threadgroup_size;
    
    HaltonSamplerPrepareForFrameUniforms uniforms{};
    uniforms.frame_size = frame_size;
    uniforms.frame_index = frame_index;
    
    dispatch(*_prepare_for_frame_kernel, threadgroups, threadgroup_size, [&](KernelArgumentEncoder &encoder) {
        encoder["uniforms"]->set_bytes(&uniforms, sizeof(HaltonSamplerPrepareForFrameUniforms));
        encoder["states"]->set_buffer(*_state_buffer);
    });
}

}