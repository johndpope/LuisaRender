#include <iostream>
#include <core/camera.h>
#include <core/material.h>
#include <core/light.h>
#include <core/integrator.h>
#include <core/string_manipulation.h>
#include <core/parser.h>

int main() {
    
    using namespace luisa;
    
    Parser parser;
    parser.parse("resources/scenes/test.scene");
    
    return 0;
}
