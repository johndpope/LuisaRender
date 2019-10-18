//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <glm/glm.hpp>
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Film) {
    
    PROPERTY(glm::uvec2, size, CoreTypeTag::INTEGER) {
        _size.x = static_cast<uint32_t>(params[0]);
        _size.y = static_cast<uint32_t>(params[1]);
    }
    
    PROPERTY(std::shared_ptr<Filter>, filter, CoreTypeTag::FILTER) {
        _filter = params[0];
    }
    
};

}
