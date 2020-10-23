#define _XOPEN_SOURCE 700
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(A)      (sizeof (A) / sizeof (*(A)))
#define TEMPNAM_TEMPLATE  "%s/%sXXXXXX"

static bool
mkstemp_close_unlink(char *buf, char const* who)
{
    int fd = mkstemp(buf);
    if (fd < 0) false;

    if (close(fd) < 0) perror(who);
    if (unlink(buf) < 0) perror(who);

    return true;
}

static char*
real_tmpnam(char* buf)
{
    static char my_buf[L_tmpnam];
    if (!buf) buf = my_buf;

    snprintf(buf, L_tmpnam, TEMPNAM_TEMPLATE, P_tmpdir, "tmp");
    bool success = mkstemp_close_unlink(buf, "tmpnam");

    return success? buf : NULL;
}

static char*
vmsprintf(size_t guess,
          const char* restrict format,
          va_list ap)
{
    char buffer[guess];

    va_list ap1;
    va_copy(ap1, ap);
    size_t needed = vsnprintf(buffer, sizeof buffer, format, ap1);

    char* result = malloc(needed);
    if (!result) goto finish;

    if (sizeof buffer < needed) {
        vsnprintf(result, needed, format, ap);
    } else {
        memcpy(result, buffer, needed);
    }

finish:
    va_end(ap1);
    return result;
}

static char*
msprintf(size_t guess,
         const char* restrict format,
         ...)
{
    va_list ap;
    va_start(ap, format);
    char* result = vmsprintf(guess, format, ap);
    va_end(ap);
    return result;
}

static char*
try_tempnam(char const* dir, char const* pfx)
{
    if (!dir) return NULL;
    if (!pfx) pfx = "tempnam";

    char* result = msprintf(256, TEMPNAM_TEMPLATE, dir, pfx);
    if (!result) return NULL;

    bool success = mkstemp_close_unlink(result, "tempnam");
    if (!success) {
        free(result);
        return NULL;
    }

    return result;
}

char*
tempnam(char const* dir, char const* pfx)
{
    char const* candidates[] = {
        dir,
        P_tmpdir,
        getenv("TMPDIR"),
        "/tmp",
        ".",
    };

    for (size_t i = 0; i < ARRAY_LEN(candidates); ++i) {
        char* result = try_tempnam(candidates[i], pfx);
        if (result) return result;
    }

    return NULL;
}

char*
tmpnam(char* buf)
{
    return real_tmpnam(buf);
}

char*
tmpnam_r(char* s)
{
    return s ? real_tmpnam(s) : NULL;
}
