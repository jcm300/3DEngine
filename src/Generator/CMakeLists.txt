cmake_minimum_required(VERSION 2.8)
use_c99()

if(UNIX)
    set(MATH_LIB m)
endif(UNIX)

add_executable(generator generator.c)
target_link_libraries(generator ${MATH_LIB})
