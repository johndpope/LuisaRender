//
// Created by Mike Smith on 2019/10/31.
//

#pragma once

#include "device.h"
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Sampler) {

protected:
    std::shared_ptr<Texture> _random_texture;

public:
    virtual void initialize(Device &device) = 0;
    virtual void prepare_for_frame(KernelDispatcher &dispatch, math::uint2 frame_size, uint frame_index, uint total_dimensions) = 0;
    virtual void generate_samples(KernelDispatcher &dispatch, Texture &random_texture, uint dimensions) = 0;
};

}
