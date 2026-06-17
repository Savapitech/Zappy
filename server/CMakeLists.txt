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

option(BUILD_TESTS "Build the Criterion unit test suite" OFF)
if(BUILD_TESTS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CRITERION REQUIRED criterion)

    list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_library(zappy_core STATIC ${SOURCES})
    target_include_directories(zappy_core PUBLIC src)
    target_compile_options(zappy_core PRIVATE -Wall -Wextra)

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/unit/*.cpp)
    add_executable(zappy_tests ${TEST_SOURCES})
    target_include_directories(zappy_tests PRIVATE src ${CRITERION_INCLUDE_DIRS})
    target_link_libraries(zappy_tests PRIVATE zappy_core ${CRITERION_LIBRARIES})
    target_link_directories(zappy_tests PRIVATE ${CRITERION_LIBRARY_DIRS})
    target_compile_options(zappy_tests PRIVATE ${CRITERION_CFLAGS_OTHER})
endif()
