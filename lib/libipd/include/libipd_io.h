#pragma once

#include <stdio.h>

// Reads a line of input on stdin. The returned string is allocated by
// `malloc` and must be freed with `free`. Returns NULL on end-of-file.
//
// ERRORS:
//  - on out-of-memory, prints a message to stderr and exits with code 1
char* read_line(void);

// Reads a line of input from the given file handle. As with
// `read_line`, the returned string is allocated by `malloc` and must be
// freed with `free`. Returns NULL on end-of-file.
//
// ERRORS:
//  - on out-of-memory, prints a message to stderr and exits with code 1
char* fread_line(FILE*);

// Like `read_line`, but prints a prompt and flushes it to ensure that
// it's displayed immediately even if it doesn't end with a newline. Be
// careful with this, as most homework assigments specify very
// particular output, and extra output will be considered a bug. The
// arguments are the same as for printf(1).
char* prompt_line(const char* format, ...)
__attribute__((format(printf, 1, 2)));

// From <stdlib.h>, but necessary for using `read_line`, `fread_line`,
// and `prompt_line` correctly.
void free(void*);


/*
 * DEBUGGING
 */

// Like printf(3), but prints to stderr instead of stdout. This exists
// mainly to be enabled as `debug` by #defining ENABLE_DEBUGF before
// including this header.
void eprintf(const char* format, ...)
__attribute__((format(printf, 1, 2)));

// Like eprintf, but is a no-op by default, and only prints if you
// #define ENABLE_DEBUGF above the #include of this file.
#ifdef ENABLE_DEBUGF
#   define  debugf           eprintf
#else
#   define  debugf(...)      do {} while (false)
#endif
