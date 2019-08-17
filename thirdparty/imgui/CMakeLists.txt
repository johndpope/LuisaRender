file(GLOB IMGUI_SOURCES
     imgui/*.cpp
     imgui/*.h)

add_library(imgui ${IMGUI_SOURCES})
