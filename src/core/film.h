//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#include "device.h"
#include "type_reflection.h"
#include "mathematics.h"

namespace luisa {

LUISA_MAKE_ERROR_TYPE(FilmError);

#define THROW_FILM_ERROR(...)  \
    LUISA_THROW_ERROR(FilmError, __VA_ARGS__)

CORE_CLASS(Film) {

protected:
    PROPERTY(math::uint2, size, CoreTypeTag::FLOAT) {
        if (params.size() != 2) {
            THROW_FILM_ERROR("expected exactly two integer values as film size.");
        }
        _size = {params[0], params[1]};
    }

public:
    void initialize(Device &device [[maybe_unused]], const CoreTypeInitializerParameterSet &param_set) override {
        if (!_decode_size(param_set)) {
            LUISA_WARNING("film size not specified, using default value (1280x720).");
            _size = {1280u, 720u};
        }
    }
    
    virtual void convert_colorspace(KernelDispatcher &dispatcher, Texture &result_texture) = 0;
    
    [[nodiscard]] math::uint2 size() const noexcept { return _size; }
};

}
