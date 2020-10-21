#pragma once

bool rtipd_test_log_check(
        bool condition,
        char const* file,
        int line);

void rtipd_test_log_error(
        char const* context,
        char const* file,
        int line);
