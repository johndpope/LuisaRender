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

class RGBFilm : public Film {

private:
    std::shared_ptr<Kernel> _convert_colorspace_kernel;

public:
    void initialize(Device &device) override;
    void convert_colorspace(KernelDispatcher &dispatch) override;
};

}

#endif
