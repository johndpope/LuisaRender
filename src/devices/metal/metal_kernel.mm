//
// Created by Mike Smith on 2019/10/24.
//

#import <MetalPerformanceShaders/MetalPerformanceShaders.h>
#import <util/string_manipulation.h>
#import "metal_kernel.h"
#import "metal_buffer.h"
#import "metal_texture.h"

namespace luisa::metal {

void MetalKernelDispatcher::operator()(Kernel &kernel, math::uint2 grids, math::uint2 grid_size, std::function<void(KernelArgumentEncoder &)> encode) {
    auto encoder = [_command_buffer computeCommandEncoder];
    auto &&metal_kernel = dynamic_cast<MetalKernel &>(kernel);
    MetalKernelArgumentEncoder metal_encoder{metal_kernel.reflection(), encoder};
    encode(metal_encoder);
    [encoder setComputePipelineState:metal_kernel.pipeline()];
    [encoder dispatchThreadgroups:MTLSizeMake(grids.x, grids.y, 1) threadsPerThreadgroup:MTLSizeMake(grid_size.x, grid_size.y, 1)];
    [encoder endEncoding];
}

std::unique_ptr<KernelArgumentProxy> MetalKernelArgumentEncoder::operator[](std::string_view name) {
    for (MTLArgument *argument in _info.arguments) {
        if (name == util::to_string(argument.name)) {
            return std::make_unique<MetalKernelArgumentProxy>(argument, _encoder);
        }
    }
    throw std::runtime_error{"argument not found in Metal compute kernel."};
}

void MetalKernelArgumentProxy::set_buffer(Buffer &buffer, size_t offset) {
    [_encoder setBuffer:dynamic_cast<MetalBuffer &>(buffer).handle() offset:offset atIndex:_argument.index];
}

void MetalKernelArgumentProxy::set_texture(Texture &texture) {
    [_encoder setTexture:dynamic_cast<MetalTexture &>(texture).handle() atIndex:_argument.index];
}

void MetalKernelArgumentProxy::set_bytes(const void *bytes, size_t size) {
    [_encoder setBytes:bytes length:size atIndex:_argument.index];
}

}
