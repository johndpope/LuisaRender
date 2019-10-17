cmake_minimum_required(VERSION 3.14)
project(LuisaRender)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(GLM_QUIET ON CACHE BOOL "" FORCE)
set(GLM_TEST_ENABLE_CXX_17 ON CACHE BOOL "" FORCE)
include_directories(thirdparty/glm)
add_subdirectory(thirdparty/glm)
link_libraries(glm_static)

set(BUILD_FRAMEWORK OFF CACHE BOOL "" FORCE)
set(ASSIMP_DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(ASSIMP_OPT_BUILD_PACKAGES OFF CACHE BOOL "" FORCE)
set(ASSIMP_ANDROID_JNIIOSYSTEM OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_COVERALLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WERROR OFF CACHE BOOL "" FORCE)
set(ASSIMP_ASAN OFF CACHE BOOL "" FORCE)
set(ASSIMP_UBSAN OFF CACHE BOOL "" FORCE)
set(SYSTEM_IRRXML OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(IGNORE_GIT_HASH OFF CACHE BOOL "" FORCE)
include_directories(thirdparty/assimp/include)
add_subdirectory(thirdparty/assimp)
link_libraries(assimp)

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
include_directories(thirdparty/assimp/contrib/rapidjson/include)

add_subdirectory(kernels)

include_directories(src)
add_subdirectory(src)
link_libraries(luisa_render)

link_libraries("-framework Foundation" "-framework Metal" "-framework Metalkit" "-framework MetalPerformanceShaders")

add_executable(LuisaRender main.cpp)
add_dependencies(LuisaRender kernels)
