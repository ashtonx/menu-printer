cmake_minimum_required(VERSION 3.14.5)
project (dietsort)

# SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread") #for boost
set(SOURCE
data_structs.hpp
parse_settings.hpp
boost_process.cpp
json.cpp
)

if(CMAKE_VERSION VERSION_GREATER 3.6)
  #add clang-tidy if available
  find_program(
    CLAN_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Patch to clang-tidy executable"
  )

  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE")
  endif()
endif()

add_executable(dietsort main.cpp ${SOURCE})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.69.0 REQUIRED COMPONENTS system)

find_package(nlohmann_json 3.7.0 REQUIRED)
# set(LIBS
# ${Boost_LIBRARIES}
# )

target_link_libraries (dietsort Boost::system nlohmann_json::nlohmann_json
  )

