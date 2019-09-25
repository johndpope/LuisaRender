#import <iostream>
#import <filesystem>

#import <MetalKit/MetalKit.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>

int main() {
    auto device = MTLCreateSystemDefaultDevice();
    std::cout << [device.name cStringUsingEncoding:NSUTF8StringEncoding] << std::endl;
    
    auto library_path = std::filesystem::absolute("kernels.metallib");
    std::cout << "Library path: " << library_path << std::endl;
    auto library = [device newLibraryWithFile:[[[NSString alloc] initWithCString:library_path.c_str()
                                                                        encoding:NSUTF8StringEncoding] autorelease]
                                        error:nullptr];
    [library autorelease];
    
    std::cout << [library.description cStringUsingEncoding:NSUTF8StringEncoding] << std::endl;
}