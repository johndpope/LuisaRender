//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <glm/glm.hpp>
#include "type_reflection.h"

namespace luisa {

core_class(Transform) {
public:
    virtual glm::mat4 at(float time) = 0;
};

}
