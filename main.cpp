#include <iostream>
#include <core/camera.h>

int main() {
    
    using namespace luisa;
    
    std::cout << is_core_type < Camera > << std::endl;
    
    std::cout << name_of_core_type_tag(Camera::base_tag) << std::endl;
    
    int a = 5;
    auto x = TypeReflectionManager::instance().create<CoreTypeTag::CAMERA>("ThinLens", a);
    auto y = TypeReflectionManager::instance().create<CoreTypeTag::CAMERA>("Perspective", a);
    
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
