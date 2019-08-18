file(GLOB IMGUI_SOURCES
     imgui/*.cpp
     imgui/*.h
     imgui/examples/imgui_impl_glfw.cpp
     imgui/examples/imgui_impl_glfw.h
     imgui/examples/imgui_impl_metal.mm
     imgui/examples/imgui_impl_metal.h)
add_definitions(-fobjc-arc)
add_library(imgui ${IMGUI_SOURCES})
