#pragma once

#include <stddef.h>

// Removes any allocation limit.
void alloc_limit_set_no_limit(void);

// Limits subsequent net allocations-less-frees to `n` bytes total.
void alloc_limit_set_peak(size_t n);

// Limits subsequent gross allocations to `n` bytes total.
void alloc_limit_set_total(size_t n);
