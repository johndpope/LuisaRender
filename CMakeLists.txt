cmake_minimum_required(VERSION 3.14)
project(Metalight)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
add_definitions(-DRESOURCE_DIR="${RESOURCE_DIR}")

link_libraries("-framework Foundation" "-framework Metal" "-framework MetalPerformanceShaders")
add_executable(Metalight main.mm resource.h ray.h intersection.h camera.h)

add_subdirectory(shaders)
add_dependencies(Metalight shaders)
