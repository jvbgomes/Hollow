cmake_minimum_required(VERSION 3.10)
project(Hollow)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system audio network)

include_directories(src)

file(GLOB SOURCES 
    "src/*.cpp"
)

add_executable(Hollow ${SOURCES})

target_link_libraries(Hollow sfml-graphics sfml-window sfml-system sfml-audio sfml-network)
