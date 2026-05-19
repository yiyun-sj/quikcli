function(add_quikcli_example name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE quikcli)
    target_compile_options(${name} PRIVATE -Wall -Wextra)
endfunction()

add_quikcli_example(basic)