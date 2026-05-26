cmake_minimum_required(VERSION 3.25)
project(zappy_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release|Debug)" FORCE)
endif()

find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE}")
endif()

set(CMAKE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 8)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(zappy_server ${SOURCES})

target_include_directories(zappy_server PRIVATE src)

target_compile_options(zappy_server PRIVATE
    -Wall -Wextra -Wunused -Wpedantic
    -Wno-unused-parameter -Wunused-result
    -Wcast-align -Wcast-qual
    -Wformat=2 -Wstrict-aliasing=0 -Wunreachable-code
    -Wno-unknown-warning-option
    -Werror=format-nonliteral -Werror=return-type -Werror=write-strings
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-O0;-g;-D DEBUG_MODE>
)

target_precompile_headers(zappy_server PRIVATE
    <vector>
    <memory>
    <string>
    <cstdint>
    <functional>
    <optional>
)
