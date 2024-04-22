project(riscv-emulator)

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_BUILD_TYPE Debug)

# set (CMAKE_C_COMPILER clang)
# set (CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer")
add_subdirectory(src)
