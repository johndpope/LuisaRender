#import <type_traits>
#import <string>
#import <chrono>
#import <vector>
#import <iostream>
#import <fstream>
#import <random>
#import <thread>
#import <condition_variable>

#import <opencv2/opencv.hpp>

#import <simd/simd.h>
#import <Metal/Metal.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>

#import "resource.h"
#import "ray.h"
#import "intersection.h"
#import "camera.h"

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
    id<MTLComputePipelineState> _accumulation_program{nullptr};
    
    MPSTriangleAccelerationStructure *_acceleration{nullptr};
    MPSRayIntersector *_ray_intersector{nullptr};
    
    id<MTLTexture> _accumulated_frame{nullptr};
    id<MTLTexture> _random_seeds{nullptr};
    id<MTLTexture> _output_frame{nullptr};
    MTLSize _output_frame_size{};
    
    mutable cv::Mat _image;
    mutable bool _image_should_update{false};
    
    void _update_image_if_needed() const {
        if (_image_should_update) {
            _image.create(static_cast<int>(_output_frame_size.height), static_cast<int>(_output_frame_size.width), CV_32FC4);
            [_accumulated_frame getBytes:_image.data
                             bytesPerRow:_output_frame_size.width * sizeof(float) * 4
                              fromRegion:MTLRegionMake2D(0, 0, _output_frame_size.width, _output_frame_size.height)
                             mipmapLevel:0];
            _image_should_update = false;
            cv::cvtColor(_image, _image, cv::COLOR_RGBA2BGR);
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
        
        NSArray < id<MTLDevice >> *devices = MTLCopyAllDevices();
        for (id<MTLDevice> d in devices) {
            if (!d.isLowPower) {
                _device = d;
                break;
            }
        }
        _command_queue = [_device newCommandQueue];
        
        auto path = get_resource_path("shaders.metallib");
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
        
        auto accumulation_function = [_library newFunctionWithName:@"accumulate"];
        [accumulation_function autorelease];
        _accumulation_program = [_device newComputePipelineStateWithFunction:accumulation_function error:nullptr];
        
        auto texture_desc = [[[MTLTextureDescriptor alloc] init] autorelease];
        texture_desc.pixelFormat = MTLPixelFormatRGBA32Float;
        texture_desc.width = width;
        texture_desc.height = height;
        texture_desc.usage = MTLTextureUsageShaderWrite;
        texture_desc.storageMode = MTLStorageModePrivate;
        _output_frame = [_device newTextureWithDescriptor:texture_desc];
        
        texture_desc.usage |= MTLTextureUsageShaderRead;
        texture_desc.storageMode = MTLStorageModeManaged;
        _accumulated_frame = [_device newTextureWithDescriptor:texture_desc];
        
        texture_desc.pixelFormat = MTLPixelFormatR32Uint;
        texture_desc.usage = MTLTextureUsageShaderRead;
        _random_seeds = [_device newTextureWithDescriptor:texture_desc];
        
        std::mt19937 random_engine{std::random_device{}()};
        std::uniform_int_distribution<uint32_t> distribution{0, static_cast<unsigned int>(width * height - 1)};
        std::vector<uint32_t> seeds(width * height);
        for (auto i = 0ul; i < width * height; i++) {
            seeds[i] = distribution(random_engine);
        }
        [_random_seeds replaceRegion:MTLRegionMake2D(0, 0, width, height)
                         mipmapLevel:0
                           withBytes:seeds.data()
                         bytesPerRow:sizeof(uint32_t) * width];
        
        _acceleration = [[MPSTriangleAccelerationStructure alloc] initWithDevice:_device];
        _ray_intersector = [[MPSRayIntersector alloc] initWithDevice:_device];
        
        std::array<simd_float4, 3> vertices{
            simd_make_float4(-0.5f, 0.25f, 0.0f, 1.0f),
            simd_make_float4(0.5f, 0.25f, 0.0f, 1.0f),
            simd_make_float4(0.0f, -0.25f, 0.0f, 1.0f),
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
    
    void render(const Camera &camera, uint32_t &frame_count, uint32_t spp) {
        
        auto command_buffer = [_command_queue commandBuffer];
        
        for (auto i = 1u; i <= spp; i++) {
            
            _dispatch_compute_program(_background_program, command_buffer, [this](id<MTLComputeCommandEncoder> encoder) {
                [encoder setTexture:_output_frame atIndex:0];
            });
            
            _dispatch_compute_program(_ray_generation_program, command_buffer, [this, &camera, t = frame_count + i - 1](id<MTLComputeCommandEncoder> encoder) {
                [encoder setBuffer:_ray_buffer offset:0 atIndex:0];
                [encoder setBytes:&camera length:sizeof(Camera) atIndex:1];
                [encoder setBytes:&t length:sizeof(uint32_t) atIndex:2];
                [encoder setTexture:_random_seeds atIndex:0];
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
            
            _dispatch_compute_program(_accumulation_program, command_buffer, [this, t = frame_count + i](id<MTLComputeCommandEncoder> encoder) {
                [encoder setTexture:_output_frame atIndex:0];
                [encoder setTexture:_accumulated_frame atIndex:1];
                [encoder setBytes:&t length:sizeof(uint32_t) atIndex:0];
            });
        }
        
        auto blit_encoder = [command_buffer blitCommandEncoder];
        [blit_encoder synchronizeTexture:_accumulated_frame slice:0 level:0];
        [blit_encoder endEncoding];
        
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
    
        _image_should_update = true;
        frame_count += spp;
    }
    
    void save(std::string_view path) const {
        _update_image_if_needed();
        cv::imwrite(std::string{path}, _image);
    }
    
    void show(std::string_view title) const {
        _update_image_if_needed();
        cv::Mat tone_mapped;
        cv::pow(_image, 1.0f / 2.2f, tone_mapped);
        cv::imshow(std::string{title}, tone_mapped);
    }
};

int main() {
    
    Camera camera{{2.0f,  0.0f, 2.0f},
                  {-1.0f, 0.0f, -1.0f}};
    camera.focus_at(std::sqrt(8.0f));
    
    RendererImpl renderer{1024, 768};
    auto count = 0u;
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        constexpr auto spp_per_batch = 16u;
        auto prev_count = count;
        renderer.render(camera, count, spp_per_batch);
        renderer.show("Image");
        auto stop = std::chrono::high_resolution_clock::now();
        using namespace std::chrono_literals;
        std::cout << "Average FPS of Frame[" << prev_count << "-" << count << "] = "
                  << 1e9f / ((stop - start) / 1ns) * spp_per_batch << std::endl;
        auto key = cv::waitKey(20);
        if (key == 'w') {
            camera.focus_at(camera._focal_distance * 1.1f);
            std::cout << "Focal distance: " << camera._focal_distance << std::endl;
            count = 0;
        } else if (key == 's') {
            camera.focus_at(camera._focal_distance / 1.1f);
            std::cout << "Focal distance: " << camera._focal_distance << std::endl;
            count = 0;
        } else if (key == '-') {
            camera._lens_radius /= 1.1f;
            std::cout << "Lens radius: " << camera._lens_radius << std::endl;
            count = 0;
        } else if (key == '=') {
            camera._lens_radius *= 1.1f;
            std::cout << "Lens radius: " << camera._lens_radius << std::endl;
            count = 0;
        } else if (key == 'q') {
            renderer.save(get_resource_path("image.exr"));
            break;
        }
        start = stop;
    }
    
    return 0;
}