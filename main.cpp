#include <iostream>
#include <luisa_render.h>

int main() {
    
    using namespace luisa;
    TypeReflectionManager::instance().print();
    
    auto device = Device::create("Metal");
    
    return 0;
}
