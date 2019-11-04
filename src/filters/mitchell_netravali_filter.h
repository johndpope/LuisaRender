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

DERIVED_CLASS(MitchellNetravaliFilter, Filter) {

private:
    std::shared_ptr<Kernel> _apply_kernel;

protected:
    PROPERTY(float, b, CoreTypeTag::FLOAT) {
        if (params.size() != 1) {
            THROW_FILTER_ERROR("expected exactly one float value as Mitchell-Netravali filter parameter 'B'.");
        }
        _b = params[0];
    }
    
    PROPERTY(float, c, CoreTypeTag::FLOAT) {
        if (params.size() != 1) {
            THROW_FILTER_ERROR("expected exactly one float value as Mitchell-Netravali filter parameter 'C'.");
        }
        _c = params[0];
    }

public:
    CREATOR("MitchellNetravali") noexcept { return std::make_shared<MitchellNetravaliFilter>(); }
    void initialize(Device &device, const CoreTypeInitializerParameterSet &param_set) override;
    void apply(KernelDispatcher &dispatch, Buffer &gather_ray_buffer, Texture &result_texture, math::uint2 frame_size, uint32_t frame_index) override;
};

}

#endif
