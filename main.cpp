#include <iostream>
#include <luisa_render.h>

int main() {
    
    using namespace luisa;
    
    Parser parser;
    auto tasks = parser.parse("resources/scenes/test.scene");
    
    return 0;
}
