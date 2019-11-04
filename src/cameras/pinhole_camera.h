//
// Created by Mike Smith on 2019/10/18.
//

#pragma once

#include <core/mathematics.h>

namespace luisa {

struct alignas(16) PinholeCameraGenerateRaysUniforms {
    math::float3 position;
    math::float3 front;
    math::float3 left;
    math::float3 up;
    math::float2 sensor_size;
    float near_plane;
    float fov;
    math::uint2 frame_size;
};

}

#ifndef DEVICE_COMPATIBLE

#include "core/camera.h"

namespace luisa {

DERIVED_CLASS(PinholeCamera, Camera) {

protected:
    
    PROPERTY(float, fov, CoreTypeTag::FLOAT) {
        _fov = params[0];
    }
    
    PROPERTY(float, lens_radius, CoreTypeTag::FLOAT) {
        _lens_radius = params[0];
    }
    
    PROPERTY(float, focal_distance, CoreTypeTag::FLOAT) {
        _focal_distance = params[0];
    }
    
public:
    
    size_t random_number_dimensions() const noexcept override {
        return 2;
    }
    
    void initialize(Device &device) override {
    
    }
    
    void generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size, float time) override {
    
    }
    
    void initialize(Device &device, const CoreTypeInitializerParameterSet &param_set) override {
    
    }
    
    CREATOR("Pinhole") {
        return std::make_shared<PinholeCamera>();
    }
    
};

}

#endif