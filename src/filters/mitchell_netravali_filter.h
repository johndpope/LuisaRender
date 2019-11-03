//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include <core/mathematics.h>

namespace luisa {

struct alignas(16) MitchellNetravaliFilterApplyUniforms {
    math::uint2 frame_size;
    uint32_t frame_index;
    float radius;
    float b;
    float c;
};

}

#ifndef DEVICE_COMPATIBLE

#include <core/filter.h>

namespace luisa {

class MitchellNetravaliFilter : public Filter {

private:
    std::shared_ptr<Kernel> _apply_kernel;
    float _b;
    float _c;

public:
    void initialize(Device &device) override;
    void apply(KernelDispatcher &dispatch, Buffer &gather_ray_buffer, Texture &result_texture, math::uint2 frame_size, uint32_t frame_index) override;
};

}

#endif
