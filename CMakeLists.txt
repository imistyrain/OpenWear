cmake_minimum_required(VERSION 2.8.11)
project(OpenWear)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(MRCVGL)
add_subdirectory(OpenWear)
add_subdirectory(sdm)

file(COPY OpenWear/model  DESTINATION ${CMAKE_BINARY_DIR}/)