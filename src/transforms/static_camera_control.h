//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include <glm/ext.hpp>
#include <core/transform.h>

namespace luisa {

DERIVED_CLASS(StaticCameraControl, Transform) {
    
    PROPERTY(glm::vec3, eye, CoreTypeTag::FLOAT) { _eye = glm::vec3{params[0], params[1], params[2]}; }
    PROPERTY(glm::vec3, look_at, CoreTypeTag::FLOAT) { _look_at = glm::vec3{params[0], params[1], params[2]}; }
    PROPERTY(glm::vec3, up, CoreTypeTag::FLOAT) { _up = glm::vec3{params[0], params[1], params[2]}; }
    
    CREATOR("StaticCameraControl") {
        
        auto control = std::make_shared<StaticCameraControl>();
        
        if (!control->_decode_eye(param_set)) { control->_eye = {}; }
        if (!control->_decode_look_at(param_set)) { control->_look_at = control->_eye + glm::vec3{0.0f, 0.0f, 1.0f}; }
        if (!control->_decode_up(param_set)) { control->_up = glm::vec3{0.0f, 1.0f, 0.0f}; }
        
        return control;
    }
    
public:
    glm::mat4 at(float time [[maybe_unused]]) noexcept override { return glm::lookAt(_eye, _look_at, _up); }

};

}
