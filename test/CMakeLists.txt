cmake_minimum_required(VERSION 3.3)
project(Test_dot_ipd C CXX)
include(.ipd/cmake/CMakeLists.txt)

add_c_test_program(try_check_int one_test.c)
add_cxx_test_program(try_catch one_test.cxx)
