cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(g_bmp VERSION 1.0)

# Add examples
add_subdirectory(examples/g_bmp_grayscale)
add_subdirectory(examples/g_bmp_greenscale)
add_subdirectory(examples/g_bmp_redscale)
