#pragma once

#include <stdbool.h>
#include <stddef.h>

// CHECK(A) checks that `A` evaluates to true. (Returns value of `A` in
// case you want it.)
//
// Example:
//
//     CHECK( isinf(1.0 / 0.0) );
//     CHECK( isnan(0.0 / 0.0) );

// The parentheses around `A` in the first argument to `libipd_do_check`
// ensure that if `A` contains a comma then it isn’t considered as
// separate arguments to `libipd_do_check`. The `#A` for the next
// argument turns into the code that you write for `A` but turned into a
// string literal. The next two arguments are filled in by the C
// preprocessor with the filename and line number where `CHECK` is used.
// A function can't find out what file it was used it, but a C
// preprocessor macro can, and then it can pass that information to a
// function to do the real work. The function is defined below.
#define CHECK(A)            libipd_do_check((A), #A, __FILE__, __LINE__)

// CHECK_CHAR(A, B)    checks that A and B evaluate to the same  char.
// CHECK_INT(A, B)     checks that A and B evaluate to the same  long.
// CHECK_UINT(A, B)    checks that A and B evaluate to the same  unsigned long.
// CHECK_DOUBLE(A, B)  checks that A and B evaluate to the same  double.
// CHECK_STRING(A, B)  checks that A and B evaluate to the same  string.
//   (CHECK_STRING checks for NULLs and considers them errors.)
//
// Examples:
//
//   char const *s = "hello, world";
//
//   CHECK_CHAR( s[5], ',' );
//   CHECK_UINT( strlen(s), 12 );
//   CHECK_STRING( s + 2, "llo, world" );
//
#define CHECK_CHAR(A,B)     DISPATCH_CHECK(char, A,B)
#define CHECK_INT(A,B)      DISPATCH_CHECK(long, A,B)
#define CHECK_LONG(A,B)     DISPATCH_CHECK(long, A,B)
#define CHECK_UINT(A,B)     DISPATCH_CHECK(size, A,B)
#define CHECK_ULONG(A,B)    DISPATCH_CHECK(size, A,B)
#define CHECK_SIZE(A,B)     DISPATCH_CHECK(size, A,B)
#define CHECK_DOUBLE(A,B)   DISPATCH_CHECK(double, A,B)
#define CHECK_STRING(A,B)   DISPATCH_CHECK(string, A,B)
#define CHECK_POINTER(A,B)  DISPATCH_CHECK(pointer, A,B)

// RUN_TEST takes a function with no arguments and no results, and
// calls it as a test. (This means it prints progress and success or
// failure information.)
#define RUN_TEST(F)         libipd_do_run_test((F),#F,__FILE__,__LINE__)

// Initializes the test system. The first check will call this
// automatically, but calling it yourself will ensure that you see the
// empty test results if your test program exits before getting to the
// its check.
void start_testing(void);


/*
 * IMPLEMENTATION DETAILS. The full API is documented above. Below this
 * point are implementation details that you don't need to understand in
 * order to use this library.
 */

// Helper for dispatching type-specific checks above to functions below.
// Note that `T` stands for “tag,” not “type,” since it might not be a type.
#define DISPATCH_CHECK(T, A, B) \
    libipd_do_check_##T((A),(B),#A,#B,__FILE__,__LINE__)


// Helper function used by `CHECK` macro above.
bool libipd_do_check(
        bool condition,         // did the check pass?
        char const* assertion,  // source code of condition checked
        char const* file,       // file name where `CHECK` was used
        int line);              // line number in `file`

// Helper function used by `CHECK_CHAR` macro above.
bool libipd_do_check_char(
        char have,
        char want,
        char const* expr_have,
        char const* expr_want,
        char const* file,
        int line);

// Helper function used by `CHECK_INT` and `CHECK_LONG` macros above.
bool libipd_do_check_long(
        long have,              // number we got
        long want,              // number we expected
        char const* expr_have,  // source expression producing `have`
        char const* expr_want,  // source expression producing `want`
        char const* file,
        int line);

// Helper function used by `CHECK_UINT`, `CHECK_ULONG`, and `CHECK_SIZE`
// macros above.
bool libipd_do_check_size(
        size_t have,
        size_t want,
        char const* expr_have,
        char const* expr_want,
        char const* file,
        int line);

// Helper function used by `CHECK_DOUBLE` macro above.
bool libipd_do_check_double(
        double have,
        double want,
        char const* expr_have,
        char const* expr_want,
        char const* file,
        int line);

// Helper function used by `CHECK_STRING` macro above.
bool libipd_do_check_string(
        char const* have,
        char const* want,
        char const* expr_have,
        char const* expr_want,
        char const* file,
        int line);

// Helper function used by `CHECK_POINTER` macro above.
bool libipd_do_check_pointer(
        void const* have,
        void const* want,
        char const* expr_have,
        char const* expr_want,
        char const* file,
        int line);

// Helper function used by `RUN_TEST` macro above.
//
bool libipd_do_run_test(
        void (*test_fn)(void),
        char const* source_expr,
        char const* file,
        int line);

// We're going to override exit(3) with a function that complains if
// it's called in the midst of a test.
#ifndef LIBIPD_RAW_EXIT
#  define exit(E)  libipd_exit_rt(E)
#endif

_Noreturn void libipd_exit_rt(int);
