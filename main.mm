#import <string>
#import <vector>
#import <iostream>
#import <fstream>
#import <type_traits>

#import <opencv2/opencv.hpp>

#import <Metal/Metal.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>

#import "resource.h"
#import "common.h"

class RendererImpl {

private:
    id<MTLDevice> _device{nullptr};
    id<MTLCommandQueue> _command_queue{nullptr};
    id<MTLLibrary> _library{nullptr};
    id<MTLComputePipelineState> _background_program{nullptr};
    
    id<MTLBuffer> _ray_buffer{nullptr};
    id<MTLBuffer> _intersection_buffer{nullptr};
    id<MTLComputePipelineState> _ray_generation_program{nullptr};
    id<MTLComputePipelineState> _intersection_program{nullptr};
    
    MPSTriangleAccelerationStructure *_acceleration{nullptr};
    MPSRayIntersector *_ray_intersector{nullptr};
    
    id<MTLTexture> _output_frame{nullptr};
    MTLSize _output_frame_size{};
    
    mutable cv::Mat _image;
    mutable bool _image_should_update{false};
    
    void _update_image_if_needed() const {
        if (_image_should_update) {
            _image.create(static_cast<int>(_output_frame_size.height), static_cast<int>(_output_frame_size.width), CV_32FC4);
            [_output_frame getBytes:_image.data
                        bytesPerRow:_output_frame_size.width * sizeof(float) * 4
                         fromRegion:MTLRegionMake2D(0, 0, _output_frame_size.width, _output_frame_size.height)
                        mipmapLevel:0];
            cv::cvtColor(_image, _image, cv::COLOR_RGBA2BGR);
            _image_should_update = false;
        }
    }
    
    template<typename Setup, std::enable_if_t<std::is_invocable_v<Setup, id<MTLComputeCommandEncoder>>, int> = 0>
    void _dispatch_compute_program(id<MTLComputePipelineState> program, id<MTLCommandBuffer> command_buffer, Setup &&setup) const {
        auto encoder = [command_buffer computeCommandEncoder];
        setup(encoder);
        auto w = program.threadExecutionWidth;
        auto h = program.maxTotalThreadsPerThreadgroup / w;
        [encoder setComputePipelineState:program];
        [encoder dispatchThreads:_output_frame_size threadsPerThreadgroup:{w, h, 1}];
        [encoder endEncoding];
    }

public:
    RendererImpl(size_t width, size_t height)
        : _output_frame_size{width, height, 1} {
        
        NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
        for (id<MTLDevice> d in devices) {
            if (!d.isLowPower) {
                _device = d;
                break;
            }
        }
        _command_queue = [_device newCommandQueue];
        
        auto path = get_resource_path("default.metallib");
        _library = [_device newLibraryWithFile:[[[NSString alloc] initWithCString:path.c_str() encoding:NSUTF8StringEncoding] autorelease]
                                         error:nullptr];
        
        auto background_function = [_library newFunctionWithName:@"background"];
        [background_function autorelease];
        _background_program = [_device newComputePipelineStateWithFunction:background_function error:nullptr];
        
        auto ray_generation_function = [_library newFunctionWithName:@"generate_ray"];
        [ray_generation_function autorelease];
        _ray_generation_program = [_device newComputePipelineStateWithFunction:ray_generation_function error:nullptr];
        
        auto intersection_function = [_library newFunctionWithName:@"handle_intersection"];
        [intersection_function autorelease];
        _intersection_program = [_device newComputePipelineStateWithFunction:intersection_function error:nullptr];
        
        auto output_frame_desc = [[[MTLTextureDescriptor alloc] init] autorelease];
        output_frame_desc.pixelFormat = MTLPixelFormatRGBA32Float;
        output_frame_desc.width = width;
        output_frame_desc.height = height;
        output_frame_desc.usage = MTLTextureUsageShaderWrite;
        output_frame_desc.storageMode = MTLStorageModeManaged;
        
        _output_frame = [_device newTextureWithDescriptor:output_frame_desc];
        
        _acceleration = [[MPSTriangleAccelerationStructure alloc] initWithDevice:_device];
        _ray_intersector = [[MPSRayIntersector alloc] initWithDevice:_device];
        
        std::array<simd_float4, 3> vertices{
            simd_make_float4(0.25f, 0.25f, 0.0f, 1.0f),
            simd_make_float4(0.75f, 0.25f, 0.0f, 1.0f),
            simd_make_float4(0.50f, 0.75f, 0.0f, 1.0f),
        };
        auto vertex_buffer = [_device newBufferWithBytes:vertices.data()
                                                  length:vertices.size() * sizeof(simd_float4)
                                                 options:MTLResourceStorageModeManaged];
        [vertex_buffer autorelease];
        std::array<uint32_t, 3> indices{0, 1, 2};
        auto index_buffer = [_device newBufferWithBytes:indices.data()
                                                 length:indices.size() * sizeof(uint32_t)
                                                options:MTLResourceStorageModeManaged];
        [index_buffer autorelease];
        _acceleration.vertexBuffer = vertex_buffer;
        _acceleration.vertexStride = sizeof(simd_float4);
        _acceleration.indexBuffer = index_buffer;
        _acceleration.indexType = MPSDataTypeUInt32;
        _acceleration.triangleCount = 1;
        [_acceleration rebuild];
        
        _ray_intersector.rayStride = sizeof(Ray);
        _ray_intersector.rayDataType = MPSRayDataTypeOriginMinDistanceDirectionMaxDistance;
        _ray_intersector.intersectionStride = sizeof(Intersection);
        _ray_intersector.intersectionDataType = MPSIntersectionDataTypeDistancePrimitiveIndexCoordinates;
        
        auto ray_count = _output_frame_size.width * _output_frame_size.height;
        _ray_buffer = [_device newBufferWithLength:ray_count * sizeof(Ray) options:MTLResourceStorageModePrivate];
        _intersection_buffer = [_device newBufferWithLength:ray_count * sizeof(Intersection) options:MTLResourceStorageModePrivate];
        
    }
    
    void render() {
        
        auto command_buffer = [_command_queue commandBuffer];
        _dispatch_compute_program(_background_program, command_buffer, [this](id<MTLComputeCommandEncoder> encoder) {
            [encoder setTexture:_output_frame atIndex:0];
        });
        
        _dispatch_compute_program(_ray_generation_program, command_buffer, [this](id<MTLComputeCommandEncoder> encoder) {
            [encoder setBuffer:_ray_buffer offset:0 atIndex:0];
        });

        [_ray_intersector encodeIntersectionToCommandBuffer:command_buffer
                                           intersectionType:MPSIntersectionTypeNearest
                                                  rayBuffer:_ray_buffer
                                            rayBufferOffset:0
                                         intersectionBuffer:_intersection_buffer
                                   intersectionBufferOffset:0
                                                   rayCount:_output_frame_size.width * _output_frame_size.height
                                      accelerationStructure:_acceleration];

        _dispatch_compute_program(_intersection_program, command_buffer, [this](id<MTLComputeCommandEncoder> encoder) {
            [encoder setTexture:_output_frame atIndex:0];
            [encoder setBuffer:_intersection_buffer offset:0 atIndex:0];
        });
        
        auto blit_encoder = [command_buffer blitCommandEncoder];
        [blit_encoder synchronizeTexture:_output_frame slice:0 level:0];
        [blit_encoder optimizeContentsForCPUAccess:_output_frame];
        [blit_encoder endEncoding];
        
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
        
        _image_should_update = true;
    }
    
    void save(std::string_view path) const {
        _update_image_if_needed();
        cv::imwrite(std::string{path}, _image);
    }
    
    void show(std::string_view title, int delay = 0) const {
        _update_image_if_needed();
        cv::imshow(std::string{title}, _image);
        cv::waitKey(delay);
    }
};

int main() {
    RendererImpl renderer{1920, 1080};
    renderer.render();
    renderer.show("Image");
    renderer.save(get_resource_path("image.exr"));
    return 0;
}