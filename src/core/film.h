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
    
    DECODER {
        if (!_decode_size(param_set)) {
            std::cerr << "Film size not defined, using default: 1280x720." << std::endl;
            _size = glm::uvec2{1280, 720};
        }
        if (!_decode_filter(param_set)) {
            std::cerr << "Filter not defined, using default: Neareast." << std::endl;
        }
    };
    
};

}
