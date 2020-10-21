cmake_minimum_required(VERSION 3.3)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.ipd/cmake")
include(IPDhelpers)
include(IPDcommands)

enable_testing()

find_local_package(Catch2 .ipd/lib/catch  VERSION 2020.1)
find_local_package(libIPD .ipd/lib/libipd VERSION 2020.3)

include_directories(src)

# vim: ft=cmake
