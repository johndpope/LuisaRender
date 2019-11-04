//
// Created by Mike Smith on 2019/10/20.
//

#pragma once

#include "film.h"

namespace luisa {

LUISA_MAKE_ERROR_TYPE(CameraError);

#define THROW_CAMERA_ERROR(...)  \
    LUISA_THROW_ERROR(CameraError, __VA_ARGS__)

CORE_CLASS(Camera) {

protected:
    PROPERTY(math::float3, position, CoreTypeTag::FLOAT) {
        if (params.size() != 3) {
            THROW_CAMERA_ERROR("expected exactly three float values as camera position.");
        }
        _position = {params[0], params[1], params[2]};
    }
    
    PROPERTY(math::float3, target, CoreTypeTag::FLOAT) {
        if (params.size() != 3) {
            THROW_CAMERA_ERROR("expected exactly three float values as camera target.");
        }
        _target = {params[0], params[1], params[2]};
    }
    
    PROPERTY(math::float3, up, CoreTypeTag::FLOAT) {
        if (params.size() != 3) {
            THROW_CAMERA_ERROR("expected exactly three float values as camera upside direction.");
        }
        _up = {params[0], params[1], params[2]};
    }

public:
    void initialize(Device &device [[maybe_unused]], const CoreTypeInitializerParameterSet &param_set) override {
        if (!_decode_position(param_set)) { THROW_CAMERA_ERROR("camera position not specified."); }
        if (!_decode_target(param_set)) { THROW_CAMERA_ERROR("camera target not specified."); }
        if (!_decode_up(param_set)) {
            LUISA_WARNING("camera upside direction not specified, using default value (0.0, 1.0, 0.0).");
            _up = {0.0f, 1.0f, 0.0f};
        }
    }
    
    [[nodiscard]] virtual size_t random_number_dimensions() const noexcept = 0;
    virtual void generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size, float time) = 0;
    
    void generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size) {
        generate_rays(dispatch, random_texture, ray_buffer, frame_size, 0.0f);
    }
};

}
