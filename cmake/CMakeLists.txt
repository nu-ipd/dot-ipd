cmake_minimum_required(VERSION 3.3)

if(NOT DOT_IPD_DIR)
    set(DOT_IPD_DIR "${PROJECT_SOURCE_DIR}/.ipd")
endif()

list(APPEND CMAKE_MODULE_PATH "${DOT_IPD_DIR}/cmake")
include(IPDhelpers)
include(IPDcommands)

find_local_package(Catch2 ${DOT_IPD_DIR}/lib/catch  VERSION 2020.2)
find_local_package(libIPD ${DOT_IPD_DIR}/lib/libipd VERSION 2020.6)

include_directories(src)

include(CTest)
include(Catch)

# vim: ft=cmake
