# Commands for defining build targets.
cmake_minimum_required(VERSION 3.10)


# Adds a C program with the given name and source files;
# sets the C language standard to C 11 and optionally
# enables sanitizers.
#
# Usage:
#
#   add_c_program(NAME SRCFILE... [OPTION...])
#
# where OPTIONs include:
#
#   ASAN                  enable address sanitizer
#   UBSAN                 enable undefined behavior sanitizer
#   DEFINES ‹A›=‹B›...    like `#define ‹A› ‹B›`...
#
function(add_c_program name)
    _ipd_add_program(${name} ${ARGN})
    target_link_libraries(${name} libipd)
endfunction(add_c_program)


# Adds a C++ program with the given name and source files; sets
# the C++ language standard and optionally enables sanitizers.
#
# Usage:
#
#   add_cxx_program(NAME SRCFILE... [OPTION...])
#
# where OPTIONs include:
#
#   CXX17                 enable C++ 2017 (instead of 2014)
#   ASAN                  enable address sanitizer
#   UBSAN                 enable undefined behavior sanitizer
#   DEFINES ‹A›=‹B›...    like `#define ‹A› ‹B›`...
#
function(add_cxx_program name)
    _ipd_add_program(${name} ${ARGN})
endfunction(add_cxx_program)


# Adds a C test program with the given name and source files.
# Options are the same as `add_c_program`. The `main()` function
# of the program must run the tests; it must exit with status
# zero if all tests pass, or non-zero if some test fails.
function(add_c_test_program name)
    add_c_program(${name} ${ARGN})
    add_test(NAME Test_${name} COMMAND ${name})
endfunction(add_c_test_program)


# Adds a C++ test program with the given name and source files.
# Options are the same as `add_cxx_program`, but the listed
# source files must not define `main()`.
function(add_cxx_test_program name)
    add_cxx_program(${name} ${ARGN})
    target_link_libraries(${name} catch)
    add_test(Test_${name} ${name})
endfunction(add_cxx_test_program)


###
### PRIVATE HELPERS
###

# Sets a CPP #define to let us know what operating system we're on.
function(_ipd_os_setup name)
    if (WIN32)
        target_compile_definitions(${name} PRIVATE IPD_WINDOWS)
    else ()
        target_compile_definitions(${name} PRIVATE IPD_UNIX)
    endif ()
endfunction(_ipd_os_setup)

# Common functionality for `add_*_program` forms.
function(_ipd_add_program target)
    cmake_parse_arguments(pa
            "CXX17;ASAN;UBSAN;"
            ""
            "DEFINES;SOURCES;"
            ${ARGN})

    add_executable(${target} ${pa_UNPARSED_ARGUMENTS} ${pa_SOURCES})
    _ipd_os_setup(${target})

    foreach(def ${pa_DEFINES})
        target_compile_definitions(${target} PRIVATE ${def})
    endforeach()

    set_property(TARGET ${target} PROPERTY C_STANDARD 11)
    set_property(TARGET ${target} PROPERTY C_STANDARD_REQUIRED On)
    set_property(TARGET ${target} PROPERTY C_EXTENSIONS Off)

    if (pa_CXX17)
        set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
    else ()
        set_property(TARGET ${target} PROPERTY CXX_STANDARD 14)
    endif ()

    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED On)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS Off)

    if (pa_ASAN)
        target_compile_options(${target} PRIVATE "-fsanitize=address")
        target_link_options(${target} PRIVATE "-fsanitize=address")
    endif ()

    if (pa_UBSAN)
        target_compile_options(${target} PRIVATE "-fsanitize=undefined")
        target_link_options(${target} PRIVATE "-fsanitize=undefined")
    endif ()
endfunction(_ipd_add_program)

# vim: ft=cmake