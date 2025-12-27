cmake_minimum_required(VERSION 3.20)
project(CrossLayerARQ VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings for high code quality
add_compile_options(-Wall)

# Optimization for performance
add_compile_options(-O3 -march=native)

# Include directories
include_directories(include)

# Find sources
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Define executable
add_executable(arq_sim ${SOURCES})