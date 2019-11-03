//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include <util/noncopyable.h>
#include "device.h"

namespace luisa {

class Filter : util::Noncopyable {

protected:
    float _radius;

public:
    virtual ~Filter() noexcept = default;
    virtual void initialize(Device &device) = 0;
    virtual void apply(KernelDispatcher &dispatch, Buffer &gather_ray_buffer, Texture &result_texture, math::uint2 frame_size, uint32_t frame_index) = 0;
    
    [[nodiscard]] float radius() const noexcept { return _radius; }
};

}
