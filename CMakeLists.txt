cmake_minimum_required(VERSION 3.14.5)
project (dietsort)
# SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-pthread") #for boost

set(SOURCE
main.hpp
parse_settings.hpp
include/json.hpp
)

add_executable(dietsort main.cpp ${SOURCE})

find_package(Boost 1.69.0 COMPONENTS system REQUIRED)

set(LIBS
${Boost_LIBRARIES}
)

target_link_libraries (dietsort ${LIBS})

