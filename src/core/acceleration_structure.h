//
// Created by Mike Smith on 2019/10/25.
//

#pragma once

#include "kernel.h"

namespace luisa {

struct AccelerationStructure : util::Noncopyable {
    virtual ~AccelerationStructure() noexcept = default;
    virtual void trace_any(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, size_t ray_count) = 0;
    virtual void trace_nearest(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, size_t ray_count) = 0;
    virtual void trace_any(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, Buffer &ray_count_buffer, size_t ray_count_buffer_offset) = 0;
    virtual void trace_nearest(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, Buffer &ray_count_buffer, size_t ray_count_buffer_offset) = 0;
};

}
