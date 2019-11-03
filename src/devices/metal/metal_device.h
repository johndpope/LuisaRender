//
// Created by Mike Smith on 2019/10/24.
//

#pragma once

#include <memory>
#include <core/device.h>

namespace luisa::metal {

DERIVED_CLASS(MetalDevice, Device) {

private:
    std::unique_ptr<struct MetalDeviceWrapper> _device_wrapper;
    std::unique_ptr<struct MetalLibraryWrapper> _library_wrapper;
    std::unique_ptr<struct MetalCommandQueueWrapper> _command_queue_wrapper;
    
    MetalDevice();

public:
    CREATOR("Metal") {
        return std::make_shared<MetalDevice>();
    }
    
    std::shared_ptr<Kernel> create_kernel(std::string_view function_name) override;
    std::shared_ptr<Texture> create_texture(math::uint2 size, TextureFormatTag format_tag, TextureAccessTag access_tag) override;
    std::shared_ptr<AccelerationStructure> create_acceleration_structure(Buffer &position_buffer, size_t stride, size_t triangle_count) override;
    std::shared_ptr<Buffer> create_buffer(size_t capacity, BufferStorageTag storage) override;
    
    void launch(std::function<void(KernelDispatcher &)> dispatch) override;
    void launch_async(std::function<void(KernelDispatcher &)> dispatch, std::function<void()> callback) override;
    
};

}
