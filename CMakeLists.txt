cmake_minimum_required(VERSION 3.22.1)

project(quikcli VERSION 0.0.1 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(quikcli INTERFACE)
target_include_directories(quikcli INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(example)
