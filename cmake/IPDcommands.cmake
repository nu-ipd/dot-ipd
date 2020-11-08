# Commands for defining build targets.
cmake_minimum_required(VERSION 3.10)

include(IPDhelpers)


# ADD_C_PROGRAM – Adds a C program target with the given name, built from the
# given source files. Sets the language to C 11 and enables undefined behavior
# sanitizer by default.
#
# ## Usage
#
# ```
# add_c_program(NAME [OPTION...] SRCFILE...)
# ```
#
# where OPTIONs include:
#
#   - ASAN     – enable address sanitizer
#   - NO_UBSAN – disable undefined behavior sanitizer
#   - DEFINES ‹A›=‹B›...
#              – like `#define ‹A› ‹B›`...
#
# ## Examples
#
# Build the program Pong from pong.c with address sanitizer enabled:
#
# ```
# add_program(Pong pong.c ASAN)
# ```
#
# Build the program `count` from the two listed source files:
#
# ```
# add_program(count
#     src/vc.c
#     src/count.c)
# ```
function(add_c_program name)
    _ipd_add_program(C ${name} ${ARGN})
    target_link_libraries(${name} ipd)
endfunction()


# Adds a C test program target with the given name and source files. Options are
# the same as `add_c_program`. The `main()` function of the program must run the
# tests; it must exit with status zero if all tests pass, or non-zero if some
# test fails.
#
# This command defines the preprocessor macro IPD_TESTING, which means your code
# can test whether it is being compiled for testing via `#ifdef`:
#
# ```c
# #ifdef IPD_TESTING
#   // ... code that will be compiled only when testing ...
# #else
#   // ... code that will be compiled only when NOT testing ...
# #endif
# ```
function(add_c_test_program name)
    add_c_program(${name} ${ARGN})
    target_compile_definitions(${name} PRIVATE IPD_TESTING)
    add_test(NAME Test_${name} COMMAND ${name})
endfunction()


# ADD_CXX_PROGRAM – Adds a C++ program target with the given name, built from
# the given source files. Sets the language to C++ 14 and enables undefined
# behavior sanitizer by default.
#
# ## Usage
#
# ```
# add_cxx_program(NAME [OPTION...] SRCFILE...)
# ```
#
# where `OPTION`s include:
#
#   - ASAN     – enable address sanitizer
#   - NO_UBSAN – disable undefined behavior sanitizer
#   - CXX17    – enable C++ 2017
#   - CXX20    – (experimental) enable C++ 2020
#   - DEFINES ‹A›=‹B›...
#              – like `#define ‹A› ‹B›`...
#
# ## Examples
#
# Build the program Pong from pong.cxx with address sanitizer enabled:
#
# ```
# add_program(Pong pong.cxx ASAN)
# ```
#
# Build the program Frogger from the three listed source files:
# against GE211 and using C++ 2017:
#
# ```
# add_program(Frogger
#     src/frogger_main.cxx
#     src/frogger_ui.cxx
#     src/frogger_model.cxx)
# ```
function(add_cxx_program name)
    _ipd_add_program(CXX ${name} ${ARGN})
endfunction()


# ADD_CXX_TEST_PROGRAM – Adds a test program target with the given name,
# built from the given source files and linked against the Catch2 test
# framework. The test framework provides a `main()` function, so you
# must not.
#
# The this command has the same options as `add_cxx_program`.
#
# This command defines the preprocessor macro IPD_TESTING, which means
# your code can test whether it is being compiled for testing via
# `#ifdef`:
#
# ```cxx
# #ifdef IPD_TESTING
# // ... code that will be compiled only when testing ...
# #else
# // ... code that will be compiled only when NOT testing ...
# #endif
# ```
function(add_cxx_test_program name)
    add_cxx_program(${name} ${ARGN})
    target_compile_definitions(${name} PRIVATE IPD_TESTING)
    target_link_libraries(${name} catch)
    catch_discover_tests(${name})
endfunction()


###
### PRIVATE HELPERS
###

# Sets a CPP #define to let us know what operating system we're on.
function(_ipd_os_setup name)
    if(WIN32)
        target_compile_definitions(${name} PRIVATE IPD_WINDOWS)
    else()
        target_compile_definitions(${name} PRIVATE IPD_UNIX)
    endif()
endfunction()


# Common functionality for `add_*_program` forms.
function(_ipd_add_program lang target)
    set(flags "ASAN;NO_UBSAN;")
    if(lang STREQUAL "CXX")
        set(cee 0)
        set(cxx 1)
        set(flags "CXX17;CXX20;${flags}")
    elseif(lang STREQUAL "C")
        set(cee 1)
        set(cxx 0)
    else()
        set(cee 1)
        set(cxx 1)
        set(flags "CXX17;CXX20;${flags}")
    endif()

    cmake_parse_arguments(pa
            "${flags}"
            ""
            "DEFINES;SOURCES;"
            ${ARGN})
    add_executable(${target} ${pa_UNPARSED_ARGUMENTS} ${pa_SOURCES})

    if(cee)
        target_supported_c_compile_options(${name} ${IPD_CFLAGS})
    endif()

    if(cxx)
        target_supported_cxx_compile_options(${name} ${IPD_CXXFLAGS})
    endif()

    if(NOT pa_NO_UBSAN)
        if(cee)
            target_supported_c_options(${name} "-fsanitize=undefined")
        endif()

        if(cxx)
            target_supported_cxx_options(${name} "-fsanitize=undefined")
        endif()
    endif(NOT pa_NO_UBSAN)

    if(pa_ASAN)
        if(cee)
            target_supported_c_options(${name} "-fsanitize=address")
        endif()

        if(cxx)
            target_supported_cxx_options(${name} "-fsanitize=address")
        endif()
    endif(pa_ASAN)

    _ipd_os_setup(${target})

    foreach(def ${pa_DEFINES})
        target_compile_definitions(${target} PRIVATE ${def})
    endforeach()

    if(cee)
        set_property(TARGET ${target} PROPERTY C_STANDARD               11)
        set_property(TARGET ${target} PROPERTY C_STANDARD_REQUIRED      On)
        set_property(TARGET ${target} PROPERTY C_EXTENSIONS             Off)
    endif()

    if(cxx)
        if(pa_CXX20)
            set_property(TARGET ${target} PROPERTY CXX_STANDARD         20)
        elseif(pa_CXX17)
            set_property(TARGET ${target} PROPERTY CXX_STANDARD         17)
        else()
            set_property(TARGET ${target} PROPERTY CXX_STANDARD         14)
        endif()
        set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED    On)
        set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS           Off)
    endif()
endfunction()

# C compilation flags we turn on automatically if available.
set(IPD_CFLAGS
        -Wall
        -Wcast-align=strict
        -Wcast-qual
        -Wdangling-else
        -Wnull-dereference
        -Wold-style-declaration
        -Wold-style-definition
        -Wshadow
        -Wtype-limits
        -Wwrite-strings
        -Werror=bool-compare
        -Werror=bool-operation
        -Werror=int-to-pointer-cast
        -Werror=return-type
        -Werror=uninitialized)

# C++ compilation flags we turn on automatically if available.
set(IPD_CXXFLAGS
        -Wall
        -Wcast-align=strict
        -Wcast-qual
        -Wdangling-else
        -Wnull-dereference
        -Wtype-limits
        -Wwrite-strings
        -Werror=bool-compare
        -Werror=bool-operation
        -Werror=int-to-pointer-cast
        -Werror=return-type
        -Werror=uninitialized)

# vim: ft=cmake
