cmake_minimum_required(VERSION 3.16)
project(scratch_phase1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

include_directories(
        ${SDL2_INCLUDE_DIRS}
)

add_executable(scratch_phase1
        main.cpp
)

target_link_libraries(scratch_phase1
        ${SDL2_LIBRARIES}
)
