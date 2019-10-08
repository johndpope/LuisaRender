//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Task) {

protected:
    PROPERTY(std::shared_ptr<Camera>, camera, CoreTypeTag::CAMERA) { _camera = params.front(); }
    PROPERTY(std::shared_ptr<Integrator>, integrator, CoreTypeTag::INTEGRATOR) { _integrator = params.front(); }
    PROPERTY(std::shared_ptr<Shape>, geometry, CoreTypeTag::SHAPE) { _geometry = params.front(); }
    PROPERTY(std::shared_ptr<Saver>, saver, CoreTypeTag::SAVER) { _saver = params.front(); }

public:

};

}


