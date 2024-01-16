cmake_minimum_required(VERSION 3.1)

project(sbox VERSION 1.0
                  DESCRIPTION "sandbox image demo"
                  LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-Wall -Wno-array-bounds -Wno-unused-function -Wno-unused-but-set-variable -O3")

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(sbox sbox.cc)

