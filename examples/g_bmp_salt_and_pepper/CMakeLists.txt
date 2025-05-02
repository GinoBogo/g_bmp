#cmake_minimum_required(VERSION 3.10)

project(g_bmp_salt_and_pepper VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../build)

add_compile_options(-Wall -Wextra -pedantic)

include_directories(
    ../../src
    ../../../g_fnn/src
)

add_executable(
    "g_bmp_salt_and_pepper"
    "../../../g_fnn/src/g_random.c"
    "../../src/g_bmp.c"
    "main.c"
)

target_link_libraries("g_bmp_salt_and_pepper" m)

# target_compile_definitions(g_bmp_salt_and_pepper PUBLIC MY_MACRO=1)
