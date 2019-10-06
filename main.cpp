#include <iostream>
#include <core/camera.h>
#include <core/material.h>
#include <core/light.h>
#include <core/integrator.h>
#include <core/string_manipulation.h>

int main() {
    
    using namespace luisa;
    
    std::cout << serialize(123, "456") << std::endl;
    
    std::cout << is_core_type<Camera> << std::endl;
    
    std::cout << name_of_core_type_tag(Camera::base_tag) << std::endl;
    std::cout << name_of_core_type_tag(tag_of_core_type_name("Camera")) << std::endl;
    
    for (auto name : TypeReflectionManager::instance().classes()) {
        std::cout << "class " << name;
        if (auto parent = TypeReflectionManager::instance().parent(name); !parent.empty()) {
            std::cout << " : public " << TypeReflectionManager::instance().parent(name);
        }
        std::cout << " {\n";
        for (auto p : TypeReflectionManager::instance().properties(name)) {
            std::cout << "    " << name_of_core_type_tag(p.second) << " " << p.first << ";\n";
        }
        std::cout << "};\n" << std::endl;
    }
    return 0;
}
