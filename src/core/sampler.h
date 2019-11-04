//
// Created by Mike Smith on 2019/10/31.
//

#pragma once

#include "device.h"
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Sampler) {

protected:
    uint32_t _current_dimension;

public:
    void initialize(Device &device [[maybe_unused]], const CoreTypeInitializerParameterSet &param_set [[maybe_unused]]) override { _current_dimension = 0u; }
    virtual void prepare_for_frame(KernelDispatcher &dispatch, math::uint2 frame_size, uint frame_index, uint total_dimensions) = 0;
    virtual void generate_samples(KernelDispatcher &dispatch, Texture &random_texture, uint dimensions) = 0;
};

}
