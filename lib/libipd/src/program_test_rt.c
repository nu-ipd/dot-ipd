#define _XOPEN_SOURCE 700
#define LIBIPD_RAW_ALLOC
#define LIBIPD_RAW_EXIT

#include "ipd.h"
#include "buffer.h"
#include "test_reporting.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define FD_COUNT          4
#define FOR_FD(I)         for (size_t I = 0; I < FD_COUNT; ++I)
#define COULD_NOT_EXEC    252
#define READ_FD_BUF_LEN   256
#define ARRAY_LEN(A)      (sizeof (A) / sizeof *(A))

static char const
temp_template[] = "/tmp/libipd_check_exec.XXXXXX";

static char* read_fd(int fd)
{
    if (lseek(fd, 0, SEEK_SET) < 0)
        goto could_not_lseek;

    struct buffer buf;
    if (!balloc(&buf, READ_FD_BUF_LEN))
        goto could_not_malloc;

    FILE* fstream = fdopen(dup(fd), "r");
    if (!fstream)
        goto could_not_fdopen;

    while(bfread(&buf, 1, fstream))
    { }

    if (!feof(fstream))
        goto could_not_fread;

    fclose(fstream);
    buf.data[buf.fill] = 0;
    return buf.data;

could_not_fread:
    fclose(fstream);

could_not_fdopen:
    free(buf.data);

could_not_malloc:
could_not_lseek:
    perror(NULL);
    return NULL;
}

static int write_range(
        int fd,
        char const* begin,
        char const* end)
{
    while (begin < end) {
        int res = write(fd, begin, end - begin);
        if (res < 0) {
            if (errno != EINTR) return res;
        } else {
            begin += res;
        }
    }

    return 0;
}

static int write_string(int fd, char const* str)
{
    return write_range(fd, str, str + strlen(str));
}

static int fput_cstrlit(FILE* fout, const char* str);

static void check_output_string(
        const char* file, int line, const char* descr,
        const char* expected, int fd, bool* saw_error)
{
    if (expected == ANY_OUTPUT) return;

    char* out = read_fd(fd);
    if (!out) {
        perror(NULL);
        return;
    }

    if (strcmp(out, expected)) {
        if (*saw_error) {
            eprintf("Additional check failure:\n");
        } else {
            rtipd_test_log_check(false, file, line);
            *saw_error = true;
        }

        eprintf("  reason: mismatch in %s\n", descr);

        eprintf("  have: ");
        fput_cstrlit(stderr, out);
        eprintf("\n");

        eprintf("  want: ");
        fput_cstrlit(stderr, expected);
        eprintf("\n");
    }

    free(out);
}

static void eprintf_argv(char const *const* argv)
{
    eprintf("  argv[]: {\n");

    while (*argv) {
        eprintf("    ");
        fput_cstrlit(stderr, *argv);
        if (argv + 1) eprintf(",");
        ++argv;
    }

    eprintf("  }\n");
}
static void do_check_exec(
        char const* whoami,
        char const* file,
        int line,
        char const* argv[],
        const char             *in,
        const char             *out,
        const char             *err,
        int                    code)
{
    int fd[FD_COUNT] = {-1};
    int res;

    FOR_FD(i) {
        char tempfile[sizeof temp_template];
        memcpy(tempfile, temp_template, sizeof tempfile);

        fd[i] = mkstemp(tempfile);
        if (fd[i] < 0) goto finish;

        unlink(tempfile);
    }

    if (in && in[0]) {
        res = write_string(fd[0], in);
        if (res < 0) goto finish;

        res = lseek(fd[0], 0, SEEK_SET);
        if (res < 0) goto finish;
    }

    pid_t pid = fork();
    if (pid < 0) goto finish;

    if (pid == 0) {
        FOR_FD(i) {
            dup2(fd[i], i);
            close(fd[i]);
        }

        execvp(argv[0], (char**)argv);

        dup2(3, 2);
        eprintf("%d %s\n", errno, strerror(errno));
        exit(COULD_NOT_EXEC);
    }

    // Parent:

    int status;
    res = waitpid(pid, &status, 0);
    if (res < 0) goto finish;

    if (WIFEXITED(status) && WEXITSTATUS(status) == COULD_NOT_EXEC) {
        rtipd_test_log_error("external program check", file, line);
        char* msg = read_fd(fd[3]);
        eprintf("%s: %s\n", argv[0], msg? msg : "could not exec");
        free(msg);
        whoami = NULL;
        goto finish;
    }

    bool saw_error = false;

    if (WIFSIGNALED(status)) {
        rtipd_test_log_error("external program check", file, line);
        saw_error = true;
        eprintf("  reason: process killed by signal %d\n", WTERMSIG(status));
        eprintf_argv(argv + 1);
    }

    check_output_string(file, line, "stdout",
            out, fd[1], &saw_error);
    check_output_string(file, line, "stderr",
            err, fd[2], &saw_error);

    int got_code = WEXITSTATUS(status);
    if ((code >= 0 && got_code != code) ||
            (code == ANY_EXIT_ERROR && got_code == 0)) {
        if (saw_error) {
            eprintf("Additional check failure:\n");
        } else {
            rtipd_test_log_check(false, file, line);
            saw_error = true;
        }

        eprintf("  reason: exit code mismatch\n");
        eprintf("  have: %d\n", got_code);
        if (code == ANY_EXIT_ERROR)
            eprintf("  want: non-zero\n");
        else
            eprintf("  want: %d\n", code);
    }

    if (!saw_error) {
        rtipd_test_log_check(true, file, line);
        whoami = NULL; // success == no one to blame
    }

finish:
    if (whoami) perror(whoami);

    FOR_FD(i) {
        if (fd[i] < 0) break;
        else close(fd[i]);
    }
}

void libipd_do_check_exec(
        char const             *file,
        int                     line,
        char const             *argv[],
        const char             *in,
        const char             *out,
        const char             *err,
        int                    code)
{
    do_check_exec("check_exec", file, line, argv, in, out, err, code);
}

void libipd_do_check_command(
        char const             *file,
        int                     line,
        char const             *command,
        const char             *in,
        const char             *out,
        const char             *err,
        int                    code)
{
    char const* argv[] = {"/bin/sh", "-c", command, NULL};
    do_check_exec("check_command", file, line, argv, in, out, err, code);
}

#define PUT(C) \
    do { \
        if (fputc(C, fout) < 0) return EOF; \
        else ++count; \
    } while (false)

static int fput_cstrlit(FILE* fout, const char* str)
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
