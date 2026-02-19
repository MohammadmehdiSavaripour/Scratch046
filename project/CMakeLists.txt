cmake_minimum_required(VERSION 3.16)
project(scratch_phase1)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

add_executable(scratch_phase1
        phase1.cpp
        scratchengine.cpp
)

target_include_directories(scratch_phase1 PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(scratch_phase1 ${SDL2_LIBRARIES})
