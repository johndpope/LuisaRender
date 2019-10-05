
//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <string>
#include <iostream>

#include <glm/glm.hpp>

#include "type_reflection.h"

namespace luisa {

core_class(Camera) {

protected:
    property(std::shared_ptr<Film>, film, CoreTypeTag::FILM);
    property(std::shared_ptr<Transform>, transform,CoreTypeTag::TRANSFORM);

public:
    [[nodiscard]] virtual glm::mat4 to_world() const noexcept = 0;
};

}
