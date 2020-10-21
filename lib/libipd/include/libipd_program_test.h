#pragma once

// void CHECK_COMMAND(
//     const char* command,
//     const char* actual_input,
//     const char* expected_stdout,
//     const char* expected_stderr,
//     int         expected_exit_code);
//
// Tests the shell command `command`.
//
// Runs the command and sends it the contents of `actual_input`
// on its standard input. The test passes if the command’s
// standard output matches `expected_stdout`, its standard error
// matches `expected_stderr`, and its exit code matches
// expected_exit_code.
//
// If `expected_stdout` is ANY_OUTPUT then stdout isn’t checked, and
// likewise if `expected_stderr` is ANY_OUTPUT then stderr isn’t
// checked. If `expected_exit_code` is ANY_EXIT the the exit code isn’t
// checked, and if it’s ANY_EXIT_ERROR then any non-zero exit code
// passes.
//
#define CHECK_COMMAND(CMD, IN, OUT, ERR, RES) \
    libipd_do_check_command(__FILE__, __LINE__, CMD, IN, OUT, ERR, RES)

// void CHECK_EXEC(
//     const char* argv[],
//     const char* actual_input,
//     const char* expected_stdout,
//     const char* expected_stderr,
//     int         expected_exit_code);
//
// Runs the program `argv[0]`, passing it the remainder of `argv`
// (up to the first NULL) as arguments.
//
// The remaining parameters and behavior are the same as
// CHECK_COMMAND.
#define CHECK_EXEC(ARGV, IN, OUT, ERR, RES) \
    libipd_do_check_exec(__FILE__, __LINE__, ARGV, IN, OUT, ERR, RES)

// Pass for `expected_stdout` and/or `expected_stderr` if you
// don’t want to check those.
#define ANY_OUTPUT      NULL

// Pass for `expected_exit_code` if you don’t care what the exit
// code is.
#define ANY_EXIT        (-2)

// Pass for `expected_exit_code` if any non-zero exit code should
// succeed and zero should fail.
#define ANY_EXIT_ERROR  (-1)

///
/// Internals
///

void libipd_do_check_command(
        const char             *file,
        int                     line,
        const char             *command,
        const char             *in,
        const char             *out,
        const char             *err,
        int                    status);

void libipd_do_check_exec(
        const char             *file,
        int                     line,
        const char             *argv[],
        const char             *in,
        const char             *out,
        const char             *err,
        int                    status);
