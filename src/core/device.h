//
// Created by Mike Smith on 2019/10/24.
//

#pragma once

#include <memory>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <utility>
#include <iostream>

#include "mathematics.h"

#include "kernel.h"
#include "texture.h"
#include "acceleration_structure.h"
#include "type_reflection.h"

namespace luisa {

CORE_CLASS(Device) {

public:
    [[nodiscard]] virtual std::shared_ptr<Kernel> create_kernel(std::string_view function_name) = 0;
    [[nodiscard]] virtual std::shared_ptr<Texture> create_texture(math::uint2 size, TextureFormatTag format_tag, TextureAccessTag access_tag) = 0;
    [[nodiscard]] virtual std::shared_ptr<Buffer> create_buffer(size_t capacity, BufferStorageTag storage) = 0;
    [[nodiscard]] virtual std::shared_ptr<AccelerationStructure> create_acceleration_structure(Buffer &position_buffer, size_t stride, size_t triangle_count) = 0;
    
    virtual void launch(std::function<void(KernelDispatcher &)> dispatch) = 0;
    virtual void launch_async(std::function<void(KernelDispatcher &)> dispatch, std::function<void()> callback) = 0;
    void launch_async(std::function<void(KernelDispatcher &)> dispatch) { launch_async(std::move(dispatch), [] {}); }
};

}
