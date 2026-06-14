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

set(CMAKE_UNITY_BUILD OFF)
#set(CMAKE_UNITY_BUILD_BATCH_SIZE 8)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

find_package(Stb REQUIRED)
find_path(MINIAUDIO_INCLUDE_DIR "miniaudio.h" REQUIRED)

if(APPLE)
    enable_language(OBJCXX)
    list(FILTER SOURCES EXCLUDE REGEX ".*WindowLinux\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*WindowLinux\\.cpp$")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Window/WindowMac.mm")
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    set(PLATFORM_LIBRARIES ${COCOA_LIBRARY} ${OPENGL_LIBRARY})
elseif(WIN32)
    enable_language(OBJCXX)
    list(FILTER SOURCES EXCLUDE REGEX ".*WindowLinux\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*WindowMac\\.cpp$")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Window/WindowWindow.cpp")
    set(PLATFORM_LIBRARIES opengl32 gdi32 ws2_32)

else()
    list(FILTER SOURCES EXCLUDE REGEX ".*WindowWindow\\.cpp$")
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)

    set(PLATFORM_LIBRARIES OpenGL::GL X11::X11)
endif()

add_executable(zappy_gui ${SOURCES})

target_include_directories(zappy_gui PRIVATE src ${Stb_INCLUDE_DIR} ${MINIAUDIO_INCLUDE_DIR})

target_link_libraries(zappy_gui PRIVATE
    ${PLATFORM_LIBRARIES}
)

target_compile_options(zappy_gui PRIVATE
    -Wall -Wextra -Wunused -Wpedantic
    -Wno-unused-parameter -Wunused-result
    -Wcast-align -Wcast-qual
    -Wformat=2 -Wstrict-aliasing=0 -Wunreachable-code
    -Wno-unknown-warning-option
    -Werror=format-nonliteral -Werror=return-type -Werror=write-strings
    -D GL_SILENCE_DEPRECATION
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
