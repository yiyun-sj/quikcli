add_executable(example example.cpp)

target_compile_options(example PUBLIC -std=c++20 -Wall -g)
target_link_libraries(example PRIVATE quikcli)
