//
// Created by Mike Smith on 2019/10/6.
//

#pragma once

#include <string>
#include <sstream>

namespace luisa {

template<typename ...Args>
std::string serialize(Args &&...args) noexcept {
    std::ostringstream ss;
    static_cast<void>((ss << ... << std::forward<Args>(args)));
    return ss.str();
}

}
