//
// Created by Mike Smith on 2019/10/5.
//

#pragma once

#include <filesystem>
#include "type_reflection.h"

namespace luisa {

core_class(Saver) {

protected:
    property(std::filesystem::path, directory);

public:
    virtual write(const std::shared_ptr<Film> &film, size_t id) = 0;
}

}
