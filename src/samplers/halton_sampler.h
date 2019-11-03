//
// Created by Mike Smith on 2019/10/31.
//

#pragma once

#include <core/mathematics.h>

namespace luisa {

struct HaltonSamplerState {
    uint32_t offset;
    uint32_t dimension;
};

struct alignas(16) HaltonSamplerPrepareUniforms {
    math::uint2 frame_size;
    uint32_t frame_index;
};

struct alignas(16) HaltonSamplerGenerateUniforms {
    math::uint2 frame_size;
    uint32_t dimensions;
};

}

#ifndef DEVICE_COMPATIBLE

#include <core/sampler.h>

namespace luisa {

class HaltonSampler : public Sampler {

private:
    std::shared_ptr<Kernel> _prepare_for_frame_kernel;
    std::shared_ptr<Kernel> _generate_samples_kernel;
    std::shared_ptr<Buffer> _state_buffer;
    math::uint2 _frame_size;

public:
    void initialize(Device &device) override;
    void generate_samples(KernelDispatcher &dispatch, Texture &random_texture, uint dimensions) override;
    void prepare_for_frame(KernelDispatcher &dispatch, math::uint2 frame_size, uint frame_index, uint total_dimensions) override;
};

}

#endif
