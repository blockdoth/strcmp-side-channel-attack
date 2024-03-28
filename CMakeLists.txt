cmake_minimum_required(VERSION 3.10)

project(side_channel_attack_poc C)

set(CMAKE_C_STANDARD 23)

set(SOURCE_FILES src/main.c)

# Add the path to raylib library if it's not in the default system path
# set(CMAKE_PREFIX_PATH "/path/to/raylib")

find_package(raylib REQUIRED)

add_executable(side_channel_attack_poc ${SOURCE_FILES})

target_link_libraries(side_channel_attack_poc PRIVATE raylib m)

target_compile_options(side_channel_attack_poc PRIVATE
        -Wall
        -Wextra
        -pedantic
        -O0
        -lm
)
