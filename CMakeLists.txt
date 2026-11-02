cmake_minimum_required(VERSION 4.1)
project(MovieMapNew)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MovieMapNew main.cpp loader.cpp
        loader.h
        similarity.h
        graph.h
        graph.cpp
        franchise.h
        tags.h
        tags.cpp
        franchise.h)

#lets code find the data folder no matter where the program is run
target_compile_definitions(MovieMapNew PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}/data/")
