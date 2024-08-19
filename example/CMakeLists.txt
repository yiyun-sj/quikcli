add_executable(example example.cpp)

target_compile_options(example PUBLIC -Wall -g)
target_link_libraries(example PRIVATE quikcli)
