#pragma once

#include <stddef.h>

#ifndef LIBIPD_RAW_ALLOC
#  define malloc   rtipd_malloc
#  define calloc   rtipd_calloc
#  define realloc  rtipd_realloc
#  define reallocf rtipd_reallocf
#  define free     rtipd_free
#endif

void *reallocf(void *ptr, size_t size);
