#pragma once

bool rtipd_test_log_check(
        bool condition,
        char const* file,
        int line);

void rtipd_test_log_error(
        char const* const file,
        int         const line,
        char const* const context,
        char const* const message);

void rtipd_test_log_perror(
        char const* const file,
        int         const line,
        char const* const context);
