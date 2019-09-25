#include <imgui.h>
#include <imgui_impl_metal.h>
#include <imgui_impl_glfw.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3.h>
#import <GLFW/glfw3native.h>
#import <QuartzCore/QuartzCore.h>

#import <random>
#import <thread>
#import <condition_variable>

#import <simd/simd.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>
#import <array>
#import <iostream>

#import "util/resource.h"
#import "util/ray.h"
#import "util/intersection.h"
#import "util/camera.h"

class RendererImpl {

private:
    id<MTLDevice> _device{nullptr};
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
    id<MTLTexture> _swap_frame{nullptr};
    id<MTLTexture> _random_seeds{nullptr};
    id<MTLTexture> _output_frame{nullptr};
    MTLSize _output_frame_size{};
    
    std::thread _random_generation_thread;
    std::atomic<bool> _ready{false};
    
    template<typename Setup, std::enable_if_t<std::is_invocable_v<Setup, id<MTLComputeCommandEncoder>>, int> = 0>
    void _dispatch_compute_program(id<MTLComputePipelineState> program, id<MTLCommandBuffer> command_buffer, Setup &&setup) const {
        auto encoder = [command_buffer computeCommandEncoder];
        setup(encoder);
        auto w = program.threadExecutionWidth;
        auto h = program.maxTotalThreadsPerThreadgroup / w;
        [encoder setComputePipelineState:program];
        [encoder dispatchThreads:_output_frame_size threadsPerThreadgroup:{8, 8, 1}];
        [encoder endEncoding];
    }

public:
    RendererImpl(id<MTLDevice> device, size_t width, size_t height)
        : _device{device}, _output_frame_size{width, height, 1} {
        
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
        
        resize(width, height);
        
    }
    
    ~RendererImpl() noexcept {
        if (_random_generation_thread.joinable()) {
            _random_generation_thread.join();
        }
    }
    
    void resize(size_t width, size_t height) {
        _output_frame_size.width = width;
        _output_frame_size.height = height;
        
        auto texture_desc = [[[MTLTextureDescriptor alloc] init] autorelease];
        texture_desc.pixelFormat = MTLPixelFormatRGBA32Float;
        texture_desc.width = width;
        texture_desc.height = height;
        texture_desc.usage = MTLTextureUsageShaderWrite;
        texture_desc.storageMode = MTLStorageModePrivate;
        _output_frame = [_device newTextureWithDescriptor:texture_desc];
        
        texture_desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        _accumulated_frame = [_device newTextureWithDescriptor:texture_desc];
        
        texture_desc.pixelFormat = MTLPixelFormatR32Uint;
        texture_desc.usage = MTLTextureUsageShaderRead;
        texture_desc.storageMode = MTLStorageModeManaged;
        
        if (_random_generation_thread.joinable()) {
            _random_generation_thread.join();
        }
        _random_seeds = [_device newTextureWithDescriptor:texture_desc];
        _ready = false;
        _random_generation_thread = std::thread{[this, width, height] {
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
            _ready = true;
        }};
        
        auto ray_count = width * height;
        _ray_buffer = [_device newBufferWithLength:ray_count * sizeof(Ray) options:MTLResourceStorageModePrivate];
        _intersection_buffer = [_device newBufferWithLength:ray_count * sizeof(Intersection) options:MTLResourceStorageModePrivate];
    }
    
    void render(id<MTLCommandBuffer> command_buffer, const Camera &camera, uint32_t &frame_count, uint32_t spp) {
        
        if (!_ready) {
            return;
        }
        
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
//                [encoder setTexture:_swap_frame atIndex:2];
                [encoder setBytes:&t length:sizeof(uint32_t) atIndex:0];
            });
            
//            auto f = _accumulated_frame;
//            _accumulated_frame = _swap_frame;
//            _swap_frame = f;
        }
        frame_count += spp;
    }
    
    id<MTLTexture> accumulated_frame() {
        return _accumulated_frame;
    }
};

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char **) {
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    
    // Setup style
    ImGui::StyleColorsDark();
    
    // Load Fonts
    io.Fonts->AddFontFromFileTTF(get_resource_path("fonts/Cousine-Regular.ttf").c_str(), 14.0f);
    
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }
    
    // Create window with graphics context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Dear ImGui GLFW+Metal example", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }
    
    NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
    auto device = devices[0];
    for (id<MTLDevice> d in devices) {
        if (!d.isLowPower) {
            device = d;
            break;
        }
    }
    
    NSLog(@"%@", device);
    
    auto frame_width = 0u, frame_height = 0u;
    glfwGetFramebufferSize(window, reinterpret_cast<int *>(&frame_width), reinterpret_cast<int *>(&frame_height));
    RendererImpl renderer{device, frame_width, frame_height};
    Camera camera;
    auto frame_count = 0u;
    
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];
    [commandQueue autorelease];
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplMetal_Init(device);
    
    NSWindow *nswin = glfwGetCocoaWindow(window);
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = device;
    layer.colorspace = CGColorSpaceCreateWithName(kCGColorSpaceLinearSRGB);
    layer.pixelFormat = MTLPixelFormatBGR10A2Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;
    
    static std::condition_variable cv;
    static std::mutex mutex;
    static auto command_count = 0u;
    
    auto renderPassDescriptor = [[MTLRenderPassDescriptor new] autorelease];
    
    // Our state
    float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        
        std::cout << "Frame = " << frame_count << std::endl;
        
        glfwPollEvents();
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        layer.drawableSize = CGSizeMake(width, height);
        id<CAMetalDrawable> drawable = [layer nextDrawable];
        
        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
        
        if (width != frame_width || height != frame_height) {
            frame_width = static_cast<uint32_t>(width);
            frame_height = static_cast<uint32_t>(height);
            renderer.resize(frame_width, frame_height);
            frame_count = 0;
        } else {
            renderer.render(commandBuffer, camera, frame_count, 4u);
            auto blit_encoder = [commandBuffer blitCommandEncoder];
            [blit_encoder copyFromTexture:renderer.accumulated_frame()
                              sourceSlice:0
                              sourceLevel:0
                             sourceOrigin:MTLOriginMake(0, 0, 0)
                               sourceSize:MTLSizeMake(frame_width, frame_height, 1)
                                toTexture:drawable.texture
                         destinationSlice:0
                         destinationLevel:0
                        destinationOrigin:MTLOriginMake(0, 0, 0)];
            [blit_encoder endEncoding];
        }
        
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        // Start the Dear ImGui frame
        ImGui_ImplMetal_NewFrame(renderPassDescriptor);
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Renderer");                          // Create a window called "Hello, world!" and append into it.
        
        if (ImGui::SliderFloat3("Camera Position", &camera._position.x, -5.0f, 5.0f)) { frame_count = 0; }
        if (ImGui::SliderFloat("Focal Distance", &camera._focal_distance, 0.1f, 10.0f)) { frame_count = 0; }
        if (ImGui::SliderFloat("Aperture Radius", &camera._lens_radius, 0.0f, 0.2f)) { frame_count = 0; }
        static auto fov = 45.0f;
        if (ImGui::SliderFloat("Field of View", &fov, 5.0f, 85.0f)) {
            camera.set_horizontal_fov(fov);
            frame_count = 0;
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        
        // Rendering
        ImGui::Render();
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);
        
        [renderEncoder endEncoding];
        
        {
            std::lock_guard<std::mutex> lock{mutex};
            command_count++;
            std::cout << "A: " << command_count << std::endl;
        }
        
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
            std::unique_lock<std::mutex> lock{mutex};
            command_count--;
            std::cout << "B: " << command_count << std::endl;
            lock.unlock();
            cv.notify_one();
        }];
        
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
    }
    
    {
        std::unique_lock<std::mutex> lock{mutex};
        cv.wait(lock, [] {
            std::cout << "C: " << command_count << std::endl;
            return command_count == 0;
        });
    }
    
    // Cleanup
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
