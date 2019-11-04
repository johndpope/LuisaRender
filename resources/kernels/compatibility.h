//
// Created by Mike Smith on 2019/11/1.
//

#pragma once

#ifdef __METAL_VERSION__

#define DEVICE_COMPATIBLE

#include <metal_stdlib>

namespace luisa::math {
using namespace metal;
}

#define constexpr constexpr constant

#else

#include <core/mathematics.h>

#define constant const
#define device
#define thread
#define kernel

namespace metal {

enum struct access {
    read, write, read_write
};

template<typename T, access mode>
struct texture2d {
    virtual luisa::math::float4 read(luisa::math::uint2 coord) = 0;
    virtual void write(luisa::math::float4 value, luisa::math::uint2 coord) = 0;
};

}

#endif
