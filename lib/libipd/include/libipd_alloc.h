#ifndef _LIBIPD_ALLOC_H_
#define _LIBIPD_ALLOC_H_

#include <stdlib.h>

#undef malloc
#undef calloc
#undef realloc
#undef reallocf
#undef free
#undef read_line
#undef fread_line
#undef prompt_line

#ifndef LIBIPD_RAW_ALLOC
#  define malloc       rtipd_malloc
#  define calloc       rtipd_calloc
#  define realloc      rtipd_realloc
#  define reallocf     rtipd_reallocf
#  define free         rtipd_free
#else
#  define read_line    read_line_raw_alloc
#  define fread_line   fread_line_raw_alloc
#  define prompt_line  prompt_line_raw_alloc
#endif

// See malloc(3), calloc(3), realloc(3), reallocf(3), and free(3).
void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void* ptr, size_t size);
void* reallocf(void* ptr, size_t size);
void free(void* ptr);

#endif // _LIBIPD_ALLOC_H_
