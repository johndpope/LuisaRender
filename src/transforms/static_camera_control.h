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
    
    DECODER override {
        if (!_decode_eye(param_set)) { _eye = {}; }
        if (!_decode_look_at(param_set)) { _look_at = _eye + glm::vec3{0.0f, 0.0f, 1.0f}; }
        if (!_decode_up(param_set)) { _up = glm::vec3{0.0f, 1.0f, 0.0f}; }
    };
    
    CREATOR("StaticCameraControl") {
        auto control = std::make_shared<StaticCameraControl>();
        control->_decode(param_set);
        return control;
    }
    
public:
    glm::mat4 at(float time [[maybe_unused]]) noexcept override { return glm::lookAt(_eye, _look_at, _up); }

};

}
