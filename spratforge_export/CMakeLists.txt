cmake_minimum_required(VERSION 3.15)
project(spratforge_export LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Enable testing" ON)

add_library(stb_impl STATIC src/stb_impl.cpp)
target_include_directories(stb_impl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)

add_subdirectory(spratgen_core)

target_link_libraries(spratgen_core PUBLIC stb_impl)

add_library(libresprite_core STATIC
    third_party/libresprite_core/src/pixel_engine.cpp
)
target_include_directories(libresprite_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libresprite_core/include
)
target_include_directories(libresprite_core SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stb
)
target_link_libraries(libresprite_core PUBLIC stb_impl)

add_executable(spratforge tools/spratforge/spratforge_main.cpp)
target_link_libraries(spratforge PRIVATE spratgen_core libresprite_core)
target_include_directories(spratforge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/spratgen)
