//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/sampler.h"

namespace luisa {

DERIVED_CLASS(HaltonSampler, Sampler) {
    CREATOR("Halton") {
        auto sampler = std::make_shared<HaltonSampler>();
        sampler->_decode(param_set);
        return sampler;
    }
};

}
