cmake_minimum_required(VERSION 3.14.5)
project (dietsort)
# SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(SOURCE_CODE)
add_executable(dietsort main.cpp)