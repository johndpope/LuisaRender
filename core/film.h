//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <glm/glm.hpp>
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Film) {

protected:
    PROPERTY(glm::uvec2, size, CoreTypeTag::INTEGER) {}
    PROPERTY(std::shared_ptr<Filter>, filter, CoreTypeTag::FILTER) {}
    
};

}
