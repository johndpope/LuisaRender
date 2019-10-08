//
// Created by Mike Smith on 2019/10/5.
//

#pragma once

#include <filesystem>
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Saver) {

protected:
    PROPERTY(std::filesystem::path, directory, CoreTypeTag::STRING) { _directory = params.front(); }

public:
    virtual void write(const std::shared_ptr<Film> &film, size_t id) = 0;
};

}
