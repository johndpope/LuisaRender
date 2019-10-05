//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <glm/glm.hpp>
#include "type_reflection.h"

namespace luisa {

core_class(Film) {

protected:
    property(glm::uvec2, size, CoreTypeTag::INTEGER){};
    property(std::shared_ptr<Filter>, filter, CoreTypeTag::FILTER);
    
};

}
