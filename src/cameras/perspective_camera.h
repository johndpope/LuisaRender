//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include "core/camera.h"

namespace luisa {

DERIVED_CLASS(PerspectiveCamera, Camera) {
    
    PROPERTY(float, fov, CoreTypeTag::FLOAT) {
        _fov = params[0];
    }
    
    PROPERTY(float, lens_radius, CoreTypeTag::FLOAT) {
        _lens_radius = params[0];
    }
    
    PROPERTY(float, focal_distance, CoreTypeTag::FLOAT) {
        _focal_distance = params[0];
    }
    
    CREATOR("Perspective") {
        auto camera = std::make_shared<PerspectiveCamera>();
        
    }

};

}


