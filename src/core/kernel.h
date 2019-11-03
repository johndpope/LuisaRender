//
// Created by Mike Smith on 2019/10/24.
//

#pragma once

#include <string_view>
#include <util/noncopyable.h>

#include "mathematics.h"
#include "buffer.h"
#include "texture.h"

namespace luisa {

struct KernelArgumentProxy : util::Noncopyable {
    virtual ~KernelArgumentProxy() = default;
    virtual void set_buffer(Buffer &buffer, size_t offset) = 0;
    void set_buffer(Buffer &buffer) { set_buffer(buffer, 0ul); }
    virtual void set_texture(Texture &texture) = 0;
    virtual void set_bytes(const void *bytes, size_t size) = 0;
};

struct KernelArgumentEncoder : util::Noncopyable {
    virtual ~KernelArgumentEncoder() noexcept = default;
    [[nodiscard]] virtual std::unique_ptr<KernelArgumentProxy> operator[](std::string_view argument_name) = 0;
};

struct Kernel : util::Noncopyable {
    virtual ~Kernel() = default;
};

struct KernelDispatcher : util::Noncopyable {
    virtual ~KernelDispatcher() noexcept = default;
    virtual void operator()(Kernel &kernel, math::uint2 threadgroups, math::uint2 threadgroup_size, std::function<void(KernelArgumentEncoder &)> encode) = 0;
};

}
