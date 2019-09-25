cmake_minimum_required(VERSION 3.14)
project(Metalight)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(kernels)

add_subdirectory(core)
link_libraries(core)

link_libraries("-framework Foundation" "-framework Metal" "-framework Metalkit" "-framework MetalPerformanceShaders")

add_executable(Metalight main.mm)
add_dependencies(Metalight kernels)
