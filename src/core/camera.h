
//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <string>
#include <iostream>

#include <glm/glm.hpp>

#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Camera) {
    
    PROPERTY(std::shared_ptr<Film>, film, CoreTypeTag::FILM) {
        _film = params[0];
    }
    
    PROPERTY(std::shared_ptr<Transform>, transform, CoreTypeTag::TRANSFORM) {
        _transform = params[0];
    }
    
    DECODER {
        if (!_decode_film(param_set)) {}
        if (!_decode_transform(param_set)) {}
    };
    
};

}
