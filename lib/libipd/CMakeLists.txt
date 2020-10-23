cmake_minimum_required(VERSION 3.10)
project(libIPD
        VERSION 2020.3.6
        DESCRIPTION "A little library for IPD"
        LANGUAGES C)

###
### MAIN LIBRARY SETUP
###

add_library(ipd
        src/alloc_rt.c
        src/eprintf.c
        src/program_test_rt.c
        src/read_line.c
        src/replace_tmpnam.c
        src/test_rt.c)

set_target_properties(ipd PROPERTIES
        C_STANDARD            11
        C_STANDARD_REQUIRED   On
        C_EXTENSIONS          Off
        VERSION               ${PROJECT_VERSION})

target_include_directories(ipd SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_include_directories(ipd PRIVATE
        include)

###
### LIBRARY INSTALLATION
###

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(config_version_cmake
        ${CMAKE_CURRENT_BINARY_DIR}/libIPD/libIPDConfigVersion.cmake)
set(config_install_dir
        ${CMAKE_INSTALL_DATADIR}/cmake/libIPD)

install(TARGETS ipd EXPORT libIPDConfig
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY   include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT      libIPDConfig
        DESTINATION ${config_install_dir})
install(FILES       ${config_version_cmake}
        DESTINATION ${config_install_dir})

export(TARGETS ipd FILE libIPDConfig.cmake)
write_basic_package_version_file(${config_version_cmake}
        VERSION         ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY   SameMinorVersion)
