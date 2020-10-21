#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_LINE_BUF_SIZE 80

#define surely_realloc  rtipd_surely_realloc
#include "read_line.inc"
#undef surely_realloc

#undef _LIBIPD_ALLOC_H_
#undef _LIBIPD_IO_H_
#define LIBIPD_RAW_ALLOC
#include "read_line.inc"
