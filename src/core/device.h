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

#include <util/string_manipulation.h>

#include "mathematics.h"
#include "kernel.h"
#include "texture.h"
#include "acceleration_structure.h"

namespace luisa {

struct DeviceError : public std::runtime_error {
    template<typename ...Args>
    DeviceError(std::string_view file, size_t line, Args &&...args) noexcept
        : std::runtime_error{util::serialize("DeviceError: ", std::forward<Args>(args)..., "  [file: \"", file, "\", line: ", line, "]")} {}
};

#define THROW_DEVICE_ERROR(...) throw DeviceError{__FILE__, __LINE__, __VA_ARGS__}

class Device : util::Noncopyable {

public:
    using DeviceCreator = std::function<std::shared_ptr<Device>()>;

private:
    inline static std::unordered_map<std::string_view, DeviceCreator> _device_creators{};

protected:
    static void _register_creator(std::string_view name, DeviceCreator creator) noexcept {
        assert(_device_creators.find(name) == _device_creators.end());
        _device_creators[name] = std::move(creator);
    }

public:
    ~Device() noexcept = default;
    
    [[nodiscard]] static std::shared_ptr<Device> create(std::string_view name) {
        auto iter = _device_creators.find(name);
        if (iter == _device_creators.end()) {
            THROW_DEVICE_ERROR("unregistered device type \"", name, "\".");
        }
        return _device_creators.at(name)();
    }
    
    [[nodiscard]] virtual std::shared_ptr<Kernel> create_kernel(std::string_view function_name) = 0;
    [[nodiscard]] virtual std::shared_ptr<Texture> create_texture(math::uint2 size, TextureFormatTag format_tag, TextureAccessTag access_tag) = 0;
    [[nodiscard]] virtual std::shared_ptr<Buffer> create_buffer(size_t capacity, BufferStorageTag storage) = 0;
    [[nodiscard]] virtual std::shared_ptr<AccelerationStructure> create_acceleration_structure(Buffer &position_buffer, size_t stride, size_t triangle_count) = 0;
    
    virtual void launch(std::function<void(KernelDispatcher &)> dispatch) = 0;
    virtual void launch_async(std::function<void(KernelDispatcher &)> dispatch, std::function<void()> callback) = 0;
    void launch_async(std::function<void(KernelDispatcher &)> dispatch) { launch_async(std::move(dispatch), [] {}); }
};

}

#define DEVICE_CREATOR(name)                                                          \
        static_assert(true);                                                          \
    private:                                                                          \
        inline static struct _reg_helper_impl {                                       \
            _reg_helper_impl() noexcept { Device::_register_creator(name, create); }  \
        } _reg_helper{};                                                              \
    public:                                                                           \
        [[nodiscard]] static std::shared_ptr<Device> create()
        