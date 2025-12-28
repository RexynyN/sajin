cmake_minimum_required(VERSION 3.28)

project(sajin)

find_package(OpenCV REQUIRED)

add_executable(sajin sajin.cpp vectorOps.cpp)

target_link_libraries(sajin ${OpenCV_LIBS})