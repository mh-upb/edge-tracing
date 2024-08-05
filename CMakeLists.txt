cmake_minimum_required(VERSION 3.0.0)
project(tracing)

set (CMAKE_BUILD_TYPE Release)

add_compile_options(-std=c++17 -Wall -O3 -march=native)

find_package(OpenCV 4 REQUIRED)

add_executable(tracing
	src/tracing.cpp
	src/EdgeProcessor.cpp
	src/EdgeMap.cpp
	src/Edges.cpp
	src/Visualizer.cpp)

target_link_libraries(tracing ${OpenCV_LIBS})