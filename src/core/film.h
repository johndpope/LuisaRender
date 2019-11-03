//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include "device.h"
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Film) {

protected:
    std::shared_ptr<Texture> _texture;

public:
    virtual void initialize(Device &device) = 0;
    virtual void convert_colorspace(KernelDispatcher &dispatcher) = 0;
    [[nodiscard]] Texture &texture() noexcept { return *_texture; }
    [[nodiscard]] math::uint2 size() const noexcept { return _texture->size(); }
};

}
