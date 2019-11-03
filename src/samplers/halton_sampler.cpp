//
// Created by Mike Smith on 2019/10/31.
//

#include "halton_sampler.h"

namespace luisa {

void HaltonSampler::initialize(Device &device) {
    _prepare_for_frame_kernel = device.create_kernel("halton_sampler_prepare_for_frame");
    _generate_samples_kernel = device.create_kernel("halton_sampler_generate_samples");
}

void HaltonSampler::generate_samples(KernelDispatcher &dispatch, Texture &random_texture, uint dimensions) {

}

void HaltonSampler::prepare_for_frame(KernelDispatcher &dispatch, math::uint2 frame_size, uint frame_index, uint total_dimensions) {

}

}