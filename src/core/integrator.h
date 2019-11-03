//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Integrator) {
    
    PROPERTY(std::shared_ptr<Sampler>, sampler, CoreTypeTag::SAMPLER) {
        _sampler = params[0];
    }
    
    PROPERTY(size_t, spp, CoreTypeTag::INTEGER) {
        _spp = params[0];
    }
    
};

}


