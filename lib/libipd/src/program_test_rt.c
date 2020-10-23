#define _XOPEN_SOURCE 700
#define LIBIPD_RAW_ALLOC
#define LIBIPD_RAW_EXIT

#include "ipd.h"
#include "buffer.h"
#include "test_reporting.h"

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define READ_FD_BUF_LEN   256
#define FD_COUNT          4
#define COULD_NOT_DUP2    250
#define COULD_NOT_CLOSE   251
#define COULD_NOT_EXEC    252

#define ARRAY_LEN(A)      (sizeof (A) / sizeof *(A))

#define FOR_ARRAY(I, A)   for (size_t I = 0; I < ARRAY_LEN(A); ++i)

#define WARN_IF(C) \
    do { \
        int warn_unless_errno_stash = errno; \
        if (C) perror("warning"); \
        errno = warn_unless_errno_stash; \
    } while (0)

typedef struct
{
    int a[FD_COUNT];
} fd_set_t;

static char const
temp_template[] = "/tmp/libipd_check_exec.XXXXXX";

static char const*
child_status_string(int status) {
    switch (status) {
    case COULD_NOT_CLOSE:
        return "could not close";
    case COULD_NOT_DUP2:
        return "could not dup2";
    case COULD_NOT_EXEC:
        return "could not exec";
    default:
        return "unknown failure";
    }
}

static char*
fdread_all(int* fd)
{
    if (lseek(*fd, 0, SEEK_SET) < 0)
        goto could_not_lseek;

    struct buffer buf;
    if (!balloc(&buf, READ_FD_BUF_LEN))
        goto could_not_malloc;

    FILE* fin = fdopen(*fd, "r");
    if (!fin)
        goto could_not_fdopen;

    // Will be closed by fclose(fin):
    *fd = -1;

    while(bfread(&buf, 1, fin))
    { }

    if (!feof(fin))
        goto could_not_fread;

    WARN_IF( fclose(fin) == EOF );

    buf.data[buf.fill] = 0;
    return buf.data;

could_not_fread:
    WARN_IF( fclose(fin) == EOF );

could_not_fdopen:
    free(buf.data);

could_not_malloc:
could_not_lseek:
    if (*fd >= 0) {
        WARN_IF( close(*fd) < 0 );
        *fd = -1;
    }

    return NULL;
}

static bool
fdwrite_string(int fd, char const* str)
{
    if (fd < 0) {
        return false;
    }

    FILE* fout = fdopen(fd, "w");
    if (! fout) {
        WARN_IF( close(fd) < 0 );
        return false;
    }

    size_t length = strlen(str);
    size_t written = fwrite(str, 1, length, fout);

    WARN_IF( fclose(fout) == EOF );

    return length == written;
}

static int fput_cstrlit(FILE* fout, char const* str);

static bool
check_output_string(
        char const *const file,
        int         const line,
        char const *const context,
        char const *const descr,
        char const *const expected,
        int        *const fd)
{
    if (expected == ANY_OUTPUT) return true;

    char* out = fdread_all(fd);
    if (!out) {
        rtipd_test_log_perror(file, line, context);
        return false;
    }

    if (strcmp(out, expected)) {
        rtipd_test_log_check(false, file, line);

        fprintf(stderr, "  reason: %s had mismatch in %s\n",
                context, descr);

        fprintf(stderr, "  have: ");
        fput_cstrlit(stderr, out);
        fprintf(stderr, "\n");

        fprintf(stderr, "  want: ");
        fput_cstrlit(stderr, expected);
        fprintf(stderr, "\n");

        free(out);
        return false;
    }

    free(out);
    return true;
}

static int
child_exec(fd_set_t* fd, const char* const argv[])
{
    FOR_ARRAY (i, fd->a) {
        if ( dup2(fd->a[i], i) < 0 ) return COULD_NOT_DUP2;
        if ( close(fd->a[i]) < 0 ) return COULD_NOT_CLOSE;
    }

    execvp(argv[0], (char**)argv);

    return COULD_NOT_EXEC;
}

static void do_check_exec(
        char const* const file,
        int         const line,
        char const* const context,
        char const* const argv[],
        char const* const in,
        char const* const out,
        char const* const err,
        int         const code)
{
    bool passed = false;
    fd_set_t fd = {{-1}};

    FOR_ARRAY (i, fd.a) {
        char tempfile[sizeof temp_template];
        memcpy(tempfile, temp_template, sizeof tempfile);

        fd.a[i] = mkstemp(tempfile);
        if (fd.a[i] < 0) goto sys_error;

        WARN_IF( unlink(tempfile) < 0 );
    }

    if (in && in[0]) {
        bool success = fdwrite_string(dup(fd.a[0]), in);
        if (! success) goto sys_error;

        int res = lseek(fd.a[0], 0, SEEK_SET);
        if (res < 0) goto sys_error;
    }

    pid_t pid = fork();
    if (pid < 0) goto sys_error;

    if (pid == 0) {
        int status = child_exec(&fd, argv);
        (void) fflush(stderr);
        WARN_IF( dup2(3, 2) < 0 );
        perror(context);
        _Exit(status);
    }

    // Parent:

    int status;
    if ( waitpid(pid, &status, 0) < 0 )
        goto sys_error;

    int got_code = WEXITSTATUS(status);

    if (WIFEXITED(status) && 
            (got_code == COULD_NOT_CLOSE ||
             got_code == COULD_NOT_DUP2 ||
             got_code == COULD_NOT_EXEC))
    {
        char* msg = fdread_all(&fd.a[3]);
        rtipd_test_log_error(file, line, context,
                             msg && *msg
                             ? msg
                             : child_status_string(got_code));
        free(msg);
        goto finish;
    }

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        rtipd_test_log_error(file, line, context, "killed by signal");
        fprintf(stderr, "  signal: %s (%d)\n", strsignal(sig), sig);
        passed = false;
    }

    passed &= check_output_string(file, line, context, "stdout", out, &fd.a[1]);
    passed &= check_output_string(file, line, context, "stderr", err, &fd.a[2]);

    if ((code >= 0 && got_code != code) ||
            (code == ANY_EXIT_ERROR && got_code == 0)) {
        rtipd_test_log_check(false, file, line);
        fprintf(stderr, "  reason: exit code mismatch\n");
        fprintf(stderr, "  have: %d\n", got_code);
        if (code == ANY_EXIT_ERROR)
            fprintf(stderr, "  want: non-zero\n");
        else
            fprintf(stderr, "  want: %d\n", code);
        passed = false;
    }

    if (passed) {
        rtipd_test_log_check(true, file, line);
    }

    goto finish;

sys_error:
    rtipd_test_log_perror(file, line, context);

finish:
    FOR_ARRAY (i, fd.a) {
        WARN_IF( fd.a[i] >= 0 && close(fd.a[i]) < 0 );
    }
}

void libipd_do_check_exec(
        char const             *file,
        int                     line,
        char const             *argv[],
        char const             *in,
        char const             *out,
        char const             *err,
        int                    code)
{
    do_check_exec(file, line, "CHECK_EXEC", argv, in, out, err, code);
}

void libipd_do_check_command(
        char const             *file,
        int                     line,
        char const             *command,
        char const             *in,
        char const             *out,
        char const             *err,
        int                    code)
{
    char const* argv[] = {"/bin/sh", "-c", command, NULL};
    do_check_exec(file, line, "CHECK_COMMAND", argv, in, out, err, code);
}

#define PUT(C) \
    do { \
        if (fputc(C, fout) < 0) return EOF; \
        else ++count; \
    } while (false)

static int fput_cstrlit(FILE* fout, char const* str)
{
    size_t count = 0;

    PUT('"');

    char c;

    while ( (c = *str++) ) {
        switch (c) {
        case '\\': case '\"':
            PUT('\\'); PUT(c); break;
        case '\a':
            PUT('\\'); PUT('a'); break;
        case '\b':
            PUT('\\'); PUT('b'); break;
        case '\f':
            PUT('\\'); PUT('f'); break;
        case '\n':
            PUT('\\'); PUT('n'); break;
        case '\r':
            PUT('\\'); PUT('r'); break;
        case '\t':
            PUT('\\'); PUT('t'); break;
        case '\v':
            PUT('\\'); PUT('v'); break;
        default:
            if (isgraph(c) || c == ' ') {
                PUT(c);
            } else {
                int res = fprintf(fout, "\\x%02x", c);
                if (res < 0) return EOF;
                else count += res;
            }
        }
    }

    PUT('"');

    return count;
}
