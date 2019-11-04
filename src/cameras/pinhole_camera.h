//
// Created by Mike Smith on 2019/10/23.
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

#include <type_traits>
#include <core/camera.h>

namespace luisa {

DERIVED_CLASS(PinholeCamera, Camera) {

private:
    std::shared_ptr<Kernel> _generate_rays_kernel;

protected:
    PROPERTY(float, fov, CoreTypeTag::FLOAT) {
        if (params.size() != 1) {
            THROW_CAMERA_ERROR("expected exactly one float value for pinhole camera fov.");
        }
        _fov = params[0];
    }

public:
    CREATOR("Pinhole") noexcept { return std::make_shared<PinholeCamera>(); }
    [[nodiscard]] size_t random_number_dimensions() const noexcept override { return 2ul; }
    void generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size, float time) override;
    void initialize(Device &device, const CoreTypeInitializerParameterSet &param_set) override;
};

}

#endif