//
// Created by Mike Smith on 2019/10/25.
//

#import "metal_acceleration_structure.h"
#import "metal_kernel.h"
#import "metal_buffer.h"

namespace luisa::metal {

void MetalAccelerationStructure::trace_any(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, size_t ray_count) {
    [_any_intersector encodeIntersectionToCommandBuffer:dynamic_cast<MetalKernelDispatcher &>(dispatch).command_buffer()
                                       intersectionType:MPSIntersectionTypeAny
                                              rayBuffer:dynamic_cast<MetalBuffer &>(ray_buffer).handle()
                                        rayBufferOffset:0u
                                     intersectionBuffer:dynamic_cast<MetalBuffer &>(intersection_buffer).handle()
                               intersectionBufferOffset:0u
                                               rayCount:ray_count
                                  accelerationStructure:_structure];
}

void MetalAccelerationStructure::trace_nearest(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, size_t ray_count) {
    [_nearest_intersector encodeIntersectionToCommandBuffer:dynamic_cast<MetalKernelDispatcher &>(dispatch).command_buffer()
                                           intersectionType:MPSIntersectionTypeNearest
                                                  rayBuffer:dynamic_cast<MetalBuffer &>(ray_buffer).handle()
                                            rayBufferOffset:0u
                                         intersectionBuffer:dynamic_cast<MetalBuffer &>(intersection_buffer).handle()
                                   intersectionBufferOffset:0u
                                                   rayCount:ray_count
                                      accelerationStructure:_structure];
}

void MetalAccelerationStructure::trace_any(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, Buffer &ray_count_buffer, size_t ray_count_buffer_offset) {
    [_any_intersector encodeIntersectionToCommandBuffer:dynamic_cast<MetalKernelDispatcher &>(dispatch).command_buffer()
                                       intersectionType:MPSIntersectionTypeAny
                                              rayBuffer:dynamic_cast<MetalBuffer &>(ray_buffer).handle()
                                        rayBufferOffset:0u
                                     intersectionBuffer:dynamic_cast<MetalBuffer &>(intersection_buffer).handle()
                               intersectionBufferOffset:0u
                                         rayCountBuffer:dynamic_cast<MetalBuffer &>(ray_count_buffer).handle()
                                   rayCountBufferOffset:ray_count_buffer_offset
                                  accelerationStructure:_structure];
}

void MetalAccelerationStructure::trace_nearest(KernelDispatcher &dispatch, Buffer &ray_buffer, Buffer &intersection_buffer, Buffer &ray_count_buffer, size_t ray_count_buffer_offset) {
    [_nearest_intersector encodeIntersectionToCommandBuffer:dynamic_cast<MetalKernelDispatcher &>(dispatch).command_buffer()
                                           intersectionType:MPSIntersectionTypeNearest
                                                  rayBuffer:dynamic_cast<MetalBuffer &>(ray_buffer).handle()
                                            rayBufferOffset:0u
                                         intersectionBuffer:dynamic_cast<MetalBuffer &>(intersection_buffer).handle()
                                   intersectionBufferOffset:0u
                                             rayCountBuffer:dynamic_cast<MetalBuffer &>(ray_count_buffer).handle()
                                       rayCountBufferOffset:ray_count_buffer_offset
                                      accelerationStructure:_structure];
}

}
