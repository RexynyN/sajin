cmake_minimum_required(VERSION 3.28)

project(saijin)

find_package(OpenCV REQUIRED)

add_executable(saijin imageShow.cpp)

target_link_libraries(saijin ${OpenCV_LIBS})