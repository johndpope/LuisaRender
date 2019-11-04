//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include <core/mathematics.h>

namespace luisa {

struct alignas(16) RGBFilmConvertColorspaceUniforms {
    math::float2 frame_size;
};

}

#ifndef DEVICE_COMPATIBLE

#include <core/film.h>

namespace luisa {

DERIVED_CLASS(RGBFilm, Film) {

private:
    std::shared_ptr<Kernel> _convert_colorspace_kernel;

public:
    CREATOR("RGB") noexcept { return std::make_shared<RGBFilm>(); }
    void initialize(Device &device, const CoreTypeInitializerParameterSet &param_set) override;
    void convert_colorspace(KernelDispatcher &dispatch, Texture &result_texture) override;
};

}

#endif
