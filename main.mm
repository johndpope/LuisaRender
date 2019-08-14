#import <string>
#import <vector>
#import <iostream>
#import <fstream>
#import <opencv2/opencv.hpp>
#import <Metal/Metal.h>

#import "resource.h"

class RendererImpl {

private:
    id<MTLDevice> _device{nullptr};
    id<MTLCommandQueue> _command_queue{nullptr};
    id<MTLLibrary> _library{nullptr};
    id<MTLComputePipelineState> _background_program{nullptr};
    
    id<MTLTexture> _output_frame{nullptr};
    MTLSize _output_frame_size{};
    
    mutable cv::Mat _image;
    mutable bool _image_should_update{false};
    
    void _update_image() const {
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
        
        auto output_frame_desc = [[[MTLTextureDescriptor alloc] init] autorelease];
        output_frame_desc.pixelFormat = MTLPixelFormatRGBA32Float;
        output_frame_desc.width = width;
        output_frame_desc.height = height;
        output_frame_desc.usage = MTLTextureUsageShaderWrite;
        output_frame_desc.storageMode = MTLStorageModeManaged;
        
        _output_frame = [_device newTextureWithDescriptor:output_frame_desc];
        
    }
    
    void render() {
        auto command_buffer = [_command_queue commandBuffer];
        auto command_encoder = [command_buffer computeCommandEncoder];
        [command_encoder setTexture:_output_frame atIndex:0];
        [command_encoder setComputePipelineState:_background_program];
        auto w = _background_program.threadExecutionWidth;
        auto h = _background_program.maxTotalThreadsPerThreadgroup / w;
        [command_encoder dispatchThreads:_output_frame_size threadsPerThreadgroup:{w, h, 1}];
        [command_encoder endEncoding];
        
        auto blit_encoder = [command_buffer blitCommandEncoder];
        [blit_encoder synchronizeTexture:_output_frame slice:0 level:0];
        [blit_encoder optimizeContentsForCPUAccess:_output_frame];
        [blit_encoder endEncoding];
        
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
    
        _image_should_update = true;
    }
    
    void save(std::string_view path) const {
        _update_image();
        cv::imwrite(std::string{path}, _image);
    }
    
    void show(std::string_view title, int delay = 0) const {
        _update_image();
        cv::imshow(std::string{title}, _image);
        cv::waitKey(delay);
    }
};

int main() {
    RendererImpl renderer{1024, 1024};
    renderer.render();
    renderer.show("Image");
    renderer.save(get_resource_path("image.exr"));
    return 0;
}