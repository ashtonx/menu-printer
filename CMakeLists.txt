cmake_minimum_required(VERSION 3.14.5)
project (dietsort)
# SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE
main.hpp
# date_processing.cpp
)

add_executable(dietsort main.cpp ${SOURCE})

# find_package(Boost REQUIRED)

set(LIBS
# ${Boost_LIBRARIES}
)

target_link_libraries (dietsort ${LIBS})

