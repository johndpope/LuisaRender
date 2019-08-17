cmake_minimum_required(VERSION 3.14)
project(Metalight)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF)

include_directories(thirdparty/glfw/include)
include_directories(thirdparty/imgui/imgui)
include_directories(gui)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(thirdparty/glfw)
link_libraries(glfw)

add_subdirectory(thirdparty/imgui)
link_libraries(imgui)

add_subdirectory(gui)
link_libraries(gui)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
add_definitions(-DRESOURCE_DIR="${RESOURCE_DIR}")

link_libraries("-framework Foundation" "-framework Metal" "-framework MetalPerformanceShaders" "-framework QuartzCore")
add_executable(Metalight main.mm resource.h ray.h intersection.h camera.h)

add_subdirectory(shaders)
add_dependencies(Metalight shaders)

add_executable(TestImGui gui.mm)
add_dependencies(TestImGui shaders)