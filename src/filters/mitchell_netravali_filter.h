//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/filter.h"

namespace luisa {

DERIVED_CLASS(MitchellNetravaliFilter, Filter) {
    
    CREATOR("MitchellNetravali") {
        auto filter = std::make_shared<MitchellNetravaliFilter>();
        if (!filter->_decode_radius(param_set)) { filter->_radius = 1.0f; }
        return filter;
    }
    
};

}


