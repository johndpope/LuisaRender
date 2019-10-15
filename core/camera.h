
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

protected:
    PROPERTY(std::shared_ptr<Film>, film, CoreTypeTag::FILM);
    PROPERTY(std::shared_ptr<Transform>, transform,CoreTypeTag::TRANSFORM);

public:
    [[nodiscard]] virtual glm::mat4 to_world() const noexcept = 0;
};

}
