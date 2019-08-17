//
// Created by Mike Smith on 2019-08-16.
//

#pragma once

#include <simd/simd.h>
#include "ray.h"

#ifdef __METAL_VERSION__
# include <metal_stdlib>
#else
# include <cmath>
#endif

template<typename T>
constexpr auto radians(T deg) {
    return deg * static_cast<T>(3.14159265358979323846264338327950288 / 180);
}

struct Camera {
    
    MPSPackedFloat3 _position;
    MPSPackedFloat3 _up;
    MPSPackedFloat3 _left;
    MPSPackedFloat3 _front;
    float _near_plane;
    float _frame_width;
    float _lens_radius;
    float _focal_distance;
    
    explicit Camera(MPSPackedFloat3 position = {0.0f, 0.0f, 1.0f}, MPSPackedFloat3 front = {0.0f, 0.0f, -1.0f}, MPSPackedFloat3 up = {0.0f, 1.0f, 0.0f},
                    float near_plane = 0.02f, float fov_h = 60.0f, float lens_radius = 0.02f, float focal_distance = 1.0f)
        : _position{position},
          _front{simd::normalize(front)},
          _left{simd::normalize(simd::cross(up, front))},
          _up{simd::normalize(simd::cross(front, simd::cross(up, front)))},
          _near_plane{near_plane},
          _frame_width{2.0f * tan(radians(0.5f * fov_h)) * _near_plane},
          _lens_radius{lens_radius},
          _focal_distance{focal_distance} {}
    
    void focus_at(float distance) {
        _focal_distance = distance;
    }
};
