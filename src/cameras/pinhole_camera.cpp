//
// Created by Mike Smith on 2019/10/23.
//

#include "pinhole_camera.h"

namespace luisa {

void PinholeCamera::generate_rays(KernelDispatcher &dispatch, Texture &random_texture, Buffer &ray_buffer, math::uint2 frame_size, float time [[maybe_unused]]) {
    
    math::uint2 threadgroup_size{32, 32};
    auto threadgroups = (frame_size + threadgroup_size - 1u) / threadgroup_size;
    
    PinholeCameraGenerateRaysUniforms uniforms;
    uniforms.position = _position;
    uniforms.fov = math::radians(_fov);
    uniforms.front = math::normalize(_target - _position);
    uniforms.left = math::normalize(math::cross(_up, uniforms.front));
    uniforms.up = math::normalize(math::cross(uniforms.front, uniforms.left));
    uniforms.frame_size = frame_size;
    uniforms.near_plane = 0.01f;
    uniforms.sensor_size = math::tan(uniforms.fov) * uniforms.near_plane * 2.0f * (math::float2(frame_size) / static_cast<float>(frame_size.y));
    
    dispatch(*_generate_rays_kernel, threadgroups, threadgroup_size, [&](KernelArgumentEncoder &encoder) {
        encoder["uniforms"]->set_bytes(&uniforms, sizeof(PinholeCameraGenerateRaysUniforms));
        encoder["rays"]->set_buffer(ray_buffer);
        encoder["random"]->set_texture(random_texture);
    });
}

void PinholeCamera::initialize(Device &device, const CoreTypeInitializerParameterSet &param_set) {
    Camera::initialize(device, param_set);
    _generate_rays_kernel = device.create_kernel("pinhole_camera_generate_rays");
    if (!_decode_fov(param_set)) {
        LUISA_WARNING("parameter fov not specified, using default value (35.0).");
        _fov = 35.0f;
    }
}

}
