//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/filter.h"

namespace luisa {

DERIVED_CLASS(BoxFilter, Filter) {
    
    CREATOR("Box") {
        auto filter = std::make_shared<BoxFilter>();
        filter->_decode(param_set);
        return filter;
    }
    
};

}


