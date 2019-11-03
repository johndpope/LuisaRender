//
// Created by Mike Smith on 2019/10/20.
//

#pragma once

#include "film.h"

namespace luisa {

CORE_CLASS(Camera) {

protected:
    PROPERTY(std::shared_ptr<Film>, film, CoreTypeTag::FILM) { _film = params[0]; }

public:
    [[nodiscard]] virtual size_t random_number_dimensions() const noexcept = 0;
    virtual void initialize(Device &device) = 0;
    virtual void generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size, float time) = 0;
    
    void generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size) {
        generate_rays(dispatch, random_texture, ray_buffer, frame_size, 0.0f);
    }
};

}
