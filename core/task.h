//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include "type_reflection.h"

namespace luisa {

core_class(Task) {

protected:
    property(std::shared_ptr<Camera>, camera, CoreTypeTag::CAMERA);
    property(std::shared_ptr<Integrator>, intergrator, CoreTypeTag::INTEGRATOR);
    property(std::shared_ptr<Shape>, geometry, CoreTypeTag::SHAPE);
    property(std::shared_ptr<Saver>, saver, CoreTypeTag::SAVER);
    
public:


};

}


