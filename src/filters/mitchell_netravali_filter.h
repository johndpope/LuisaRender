//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/filter.h"

namespace luisa {

DERIVED_CLASS(MitchellNetravaliFilter, Filter) {
    
    CREATOR("MitchellNetravali") {
        auto filter = std::make_shared<MitchellNetravaliFilter>();
        filter->_decode(param_set);
        return filter;
    }
    
};

}
