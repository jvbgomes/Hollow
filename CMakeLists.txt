cmake_minimum_required(VERSION 3.16)
project(Hollow)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS "/run/host/usr/lib64/cmake/SFML")
    set(SFML_DIR "/run/host/usr/lib64/cmake/SFML")
else()
    set(SFML_DIR "/usr/lib64/cmake/SFML")
endif()
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Hollow ${SOURCES})

target_include_directories(Hollow PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(Hollow PRIVATE -I/usr/include)

target_link_libraries(Hollow sfml-graphics sfml-window sfml-system)