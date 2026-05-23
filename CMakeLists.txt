cmake_minimum_required(VERSION 3.24)
project(quikcli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(quikcli INTERFACE)
target_include_directories(quikcli INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(quikcli INTERFACE cxx_std_20)

option(QUIKCLI_BUILD_TESTS    "Build tests"    OFF)
option(QUIKCLI_BUILD_EXAMPLES "Build examples" OFF)

include(FetchContent)
if(QUIKCLI_BUILD_TESTS)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.5.2)
    FetchContent_MakeAvailable(doctest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(QUIKCLI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
