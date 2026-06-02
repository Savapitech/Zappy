cmake_minimum_required(VERSION 3.25)
project(zappy_gui LANGUAGES CXX)

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

set(UTILS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Utils")
set(STB_IMAGE_URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
set(STB_IMAGE_DEST "${UTILS_DIR}/stb_image.h")

file(MAKE_DIRECTORY "${UTILS_DIR}")

if (NOT EXISTS "${STB_IMAGE_DEST}")
    message(STATUS "Download stb_image.h from GitHub...")
    file(DOWNLOAD "${STB_IMAGE_URL}" "${STB_IMAGE_DEST}"
         STATUS download_status
         SHOW_PROGRESS)

    list (GET download_status 0 status_code)
    if (NOT status_code EQUAL 0)
        message(FATAL_ERROR "Error while downloading stb_image.h: ${download_status}")
    endif()
endif()

add_executable(zappy_gui ${SOURCES})

target_include_directories(zappy_gui PRIVATE src)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

target_link_libraries(zappy_gui PRIVATE
    OpenGL::GL
    X11::X11
)

target_compile_options(zappy_gui PRIVATE
    -Wall -Wextra -Wunused -Wpedantic
    -Wno-unused-parameter -Wunused-result
    -Wcast-align -Wcast-qual
    -Wformat=2 -Wstrict-aliasing=0 -Wunreachable-code
    -Wno-unknown-warning-option
    -Werror=format-nonliteral -Werror=return-type -Werror=write-strings
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-O0;-g;-D DEBUG_MODE>
)

target_precompile_headers(zappy_gui PRIVATE
    <vector>
    <memory>
    <string>
    <cstdint>
    <functional>
    <optional>
)