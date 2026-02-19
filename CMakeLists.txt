cmake_minimum_required(VERSION 4.0)
project(Scratch046)

set(CMAKE_CXX_STANDARD 14)

add_executable(Scratch046
        phase2.cpp)
target_link_libraries(${PROJECT_NAME} -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer)
