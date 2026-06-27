cmake_minimum_required(VERSION 3.16)
project(zappy_gui_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Only the pure, render-free part of the GUI is pulled in: the protocol parser.
# Everything math-related is header-only.
add_executable(zappy_gui_tests
    ${TEST_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/Network/NetworkManager.cpp"
)

target_include_directories(zappy_gui_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    ${CRITERION_INCLUDE_DIRS}
)
target_link_libraries(zappy_gui_tests PRIVATE ${CRITERION_LIBRARIES})
target_link_directories(zappy_gui_tests PRIVATE ${CRITERION_LIBRARY_DIRS})
target_compile_options(zappy_gui_tests PRIVATE ${CRITERION_CFLAGS_OTHER})
