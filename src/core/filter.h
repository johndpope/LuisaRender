//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Filter) {
    
    PROPERTY(float, radius, CoreTypeTag::FLOAT) {
        _radius = params[0];
    }
    
    DECODER {
        if (!_decode_radius(param_set)) { _radius = 1.0f; }
    };

};

}
