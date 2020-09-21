#define LIBIPD_RAW_EXIT
#include "libipd_test.h"
#include "libipd_io.h"

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool atexit_installed = false;
static bool tests_enabled = false;
static int pass_count = 0;
static int fail_count = 0;

static void print_test_results(void)
{
    int check_count = pass_count + fail_count;
    FILE* fout = fail_count? stderr : stdout;

    fprintf(fout, "\n");

    if (! check_count) {
        fprintf(fout, "No checks.\n");
    } else if (pass_count == 0 || fail_count == 0) {
        const char* descr = pass_count? "passed" : "failed";

        switch (check_count) {
            case 1:
                fprintf(fout, "The only check %s.\n", descr);
                break;
            case 2:
                fprintf(fout, "Both checks %s.\n", descr);
                break;
            default:
                fprintf(fout, "All %d checks %s.\n", check_count, descr);
                break;
        }
    } else {
        fprintf(fout, "%d of %d checks passed.\n",
                pass_count, check_count);
    }
}

static void exit_hook_function(void)
{
    if (tests_enabled) {
        print_test_results();

        if (fail_count) {
            _exit(fail_count);
        }
    }
}

static void install_atexit(void)
{
    if (atexit_installed) return;

    if (atexit(&exit_hook_function)) {
        perror("atexit");
        exit(10);
    }

    atexit_installed = true;
}

void start_testing(void)
{
    install_atexit();
    tests_enabled = true;
}

bool log_check(bool condition, const char* file, int line)
{
    start_testing();

    if (condition) {
        ++pass_count;
    } else {
        ++fail_count;
        eprintf("\nCheck failed (%s:%d):\n", file, line);
    }

    return condition;
}

static const char* c_escape_of_char(char c) {
    switch (c) {
        case '\a': return "a";
        case '\b': return "b";
        case '\f': return "f";
        case '\n': return "n";
        case '\r': return "r";
        case '\t': return "t";
        case '\v': return "v";
        case '\\': return "\\";
        default:   return NULL;
    }
}

static void eprintf_escaped_char(char c, char quote)
{
    const char* escaped = c_escape_of_char(c);

    if (escaped)
        eprintf("\\%s", escaped);
    else if (c == quote)
        eprintf("\\%c", c);
    else if (isgraph(c) || c == ' ')
        eprintf("%c", c);
    else
        eprintf("\\x%02x", (unsigned char) c);
}

static void eprintf_char_literal(char c) {
    if (c) {
        eprintf("'");
        eprintf_escaped_char(c, '\'');
        eprintf("'");
    } else {
        eprintf("0");
    }
}

static void eprintf_string_literal(const char* s) {
    if (s) {
        eprintf("\"");
        for ( ; *s; ++s) eprintf_escaped_char(*s, '"');
        eprintf("\"");
    } else {
        eprintf("(null)");
    }
}

bool libipd_do_check(
        bool condition,
        const char* assertion,
        const char* file,
        int line)
{
    if (log_check(condition, file, line)) return true;
    eprintf("  assertion: %s\n", assertion);
    return false;
}

bool libipd_do_check_char(
        char have,
        char want,
        const char* expr_have,
        const char* expr_want,
        const char* file,
        int line)
{
    if (log_check(have == want, file, line)) return true;

    eprintf("  have: ");
    eprintf_char_literal(have);
    eprintf("  (from: %s)\n", expr_have);

    eprintf("  want: ");
    eprintf_char_literal(want);
    eprintf("  (from: %s)\n", expr_want);

    return false;
}

bool libipd_do_check_long(
        long have,
        long want,
        const char* expr_have,
        const char* expr_want,
        const char* file,
        int line)
{
    if (log_check(have == want, file, line)) return true;
    eprintf("  have: %ld  (from: %s)\n", have, expr_have);
    eprintf("  want: %ld  (from: %s)\n", want, expr_want);
    return false;
}

bool libipd_do_check_size(
        size_t have,
        size_t want,
        const char* expr_have,
        const char* expr_want,
        const char* file,
        int line)
{
    if (log_check(have == want, file, line)) return true;
    eprintf("  have: %zu  (from: %s)\n", have, expr_have);
    eprintf("  want: %zu  (from: %s)\n", want, expr_want);
    return false;
}

bool libipd_do_check_double(
        double have,
        double want,
        const char* expr_have,
        const char* expr_want,
        const char* file,
        int line)
{
    if (log_check(have == want, file, line)) return true;
    eprintf("  have: %f  (from: %s)\n", have, expr_have);
    eprintf("  want: %f  (from: %s)\n", want, expr_want);
    return false;
}

bool libipd_do_check_string(
        const char* have,
        const char* want,
        const char* expr_have,
        const char* expr_want,
        const char* file,
        int line)
{
    if (log_check(have && want && strcmp(have, want) == 0, file, line))
        return true;

    if (!want) {
        eprintf("Fatal Error:\n");
        eprintf("The second argument to CHECK_POINTER cannot be NULL.\n");
        eprintf("If NULL is correct, use CHECK_POINTER. See the manual\n");
        eprintf("page for CHECK(3) for additional information.");
        abort();
    }

    eprintf("  have: ");
    eprintf_string_literal(have);
    eprintf("  (from: %s)\n", expr_have);

    eprintf("  want: ");
    eprintf_string_literal(want);
    eprintf("  (from: %s)\n", expr_want);

    return false;
}

bool libipd_do_check_pointer(
        const void* have,
        const void* want,
        const char* expr_have,
        const char* expr_want,
        const char* file,
        int line)
{
    if (log_check(have == want, file, line)) return true;
    eprintf("  have: %p  (from: %s)\n", have, expr_have);
    eprintf("  want: %p  (from: %s)\n", want, expr_want);
    return false;
}

_Noreturn void libipd_exit_rt(int result)
{
    if (tests_enabled) {
        print_test_results();
        tests_enabled = false;
        eprintf("libipd: exit(%d) while testing\n", result);
        if (result == 0) result = fail_count;
    }

    exit(result);
}
