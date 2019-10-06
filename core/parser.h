//
// Created by Mike Smith on 2019/10/5.
//

#pragma once

#include <vector>
#include <variant>
#include "type_reflection.h"

namespace luisa {

namespace _impl {

}


using CreatedCoreTypeVector =

class Parser {

private:
    size_t _curr_row;
    size_t _curr_col;
    std::string _source;
    std::unordered_map<std::string_view, CoreTypeVariant> _created;

public:


};

}
