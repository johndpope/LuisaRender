//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include <util/noncopyable.h>

#include "type_reflection.h"
#include "device.h"

namespace luisa {

LUISA_MAKE_ERROR_TYPE(FilterError);

#define THROW_FILTER_ERROR(...)  \
    LUISA_THROW_ERROR(FilterError, __VA_ARGS__)

CORE_CLASS(Filter) {

protected:
    PROPERTY(float, radius, CoreTypeTag::FLOAT) {
        if (params.size() != 1) {
            THROW_FILTER_ERROR("expected exactly one float value as filter radius.");
        }
        _radius = params[0];
    }

public:
    void initialize(Device &device [[maybe_unused]], const CoreTypeInitializerParameterSet &param_set) override {
        if (!_decode_radius(param_set)) {
            LUISA_WARNING("filter radius not specified, using default value (1.0).");
            _radius = 1.0f;
        }
    }
    
    virtual void apply(KernelDispatcher &dispatch, Buffer &gather_ray_buffer, Texture &result_texture, math::uint2 frame_size, uint32_t frame_index) = 0;
    
    [[nodiscard]] float radius() const noexcept { return _radius; }
};

}
