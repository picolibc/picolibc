/*
 * Flushing a non-seekable input stream must be a successful no-op.
 *
 * POSIX.1-2024 fflush() only sets the file offset of a reading stream
 * "if the file is one capable of seeking"; for pipes and other
 * non-seekable backends there is nothing to synchronize, so neither the
 * buffered read-ahead nor an ungetc() pushback may be lost and the call
 * must not fail.  Closing such a stream after a partial read must
 * succeed.  Read/write direction changes must not turn buffered input
 * into pending output.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int errors;

static void
fail(const char *what)
{
    printf("%s failed\n", what);
    errors++;
}

static FILE *
pipe_read(const char *data, int fds[2])
{
    if (pipe(fds) != 0)
        return NULL;
    if (write(fds[1], data, strlen(data)) != (ssize_t)strlen(data)) {
        close(fds[0]);
        close(fds[1]);
        return NULL;
    }
    close(fds[1]);
    return fdopen(fds[0], "r");
}

static void
test_read_ahead_kept(void)
{
    int   fds[2];
    char  buf[8];
    FILE *f = pipe_read("abcdef", fds);

    if (!f) {
        fail("read-ahead: setup");
        return;
    }
    if (fgetc(f) != 'a') {
        fail("read-ahead: first char");
        fclose(f);
        return;
    }
    if (fflush(f) != 0)
        fail("read-ahead: fflush must succeed on a pipe");
    if (ferror(f) != 0)
        fail("read-ahead: ferror set by fflush");
    if (fread(buf, 1, 5, f) != 5 || memcmp(buf, "bcdef", 5) != 0)
        fail("read-ahead: buffered input lost");
    if (fclose(f) != 0)
        fail("read-ahead: fclose");
}

static void
test_pushback_kept(void)
{
    int   fds[2];
    FILE *f = pipe_read("abcdef", fds);

    if (!f) {
        fail("pushback: setup");
        return;
    }
    if (fgetc(f) != 'a' || ungetc('Z', f) != 'Z') {
        fail("pushback: setup chars");
        fclose(f);
        return;
    }
    if (fflush(f) != 0)
        fail("pushback: fflush must succeed on a pipe");

    int char_one, char_two;

    char_one = fgetc(f);
    char_two = fgetc(f);

    /*
     * POSIX allows fflush to either drop or retain the pending ungetc
     * on this un-seekable file. Allow either behavior.
     */
    if ((char_one == 'Z' && char_two == 'b') || (char_one == 'b' && char_two == 'c'))
        ;
    else
        fail("pushback: ungetc lost across fflush");
    fclose(f);

    f = pipe_read("abcdef", fds);
    if (!f) {
        fail("pushback(fresh): setup");
        return;
    }
    if (ungetc('x', f) != 'x') {
        fail("pushback(fresh): ungetc");
        fclose(f);
        return;
    }
    if (fflush(f) != 0)
        fail("pushback(fresh): fflush must succeed on a pipe");

    char_one = fgetc(f);
    char_two = fgetc(f);

    /*
     * POSIX allows fflush to either drop or retain the pending ungetc
     * on this un-seekable file. Allow either behavior.
     */
    if ((char_one == 'x' && char_two == 'a') || (char_one == 'a' && char_two == 'b'))
        ;
    else
        fail("pushback(fresh): ungetc lost across fflush");
    fclose(f);
}

static void
test_close_partial(void)
{
    int   fds[2];
    FILE *f = pipe_read("abcdef", fds);

    if (!f) {
        fail("close: setup");
        return;
    }
    if (fgetc(f) != 'a') {
        fail("close: first char");
        fclose(f);
        return;
    }
    if (fclose(f) != 0)
        fail("close: fclose after partial read must succeed");
}

static void
test_fseek_still_fails(void)
{
    int   fds[2];
    FILE *f = pipe_read("abcdef", fds);

    if (!f) {
        fail("fseek: setup");
        return;
    }
    if (fgetc(f) != 'a') {
        fail("fseek: first char");
        fclose(f);
        return;
    }
    errno = 0;
    if (fseek(f, 100, SEEK_SET) == 0)
        fail("fseek: seek on a pipe must fail");
    if (fgetc(f) != 'b')
        fail("fseek: buffered input lost");
    fclose(f);
}

/* cookie with read and write but no seek support */
static size_t cookie_in, cookie_out_len;
static char   cookie_out[16];

static ssize_t
cookie_read(void *cookie, char *buf, size_t n)
{
    static const char data[] = "ABCDEF";
    size_t            left = strlen(data) - cookie_in;

    (void)cookie;
    if (n > left)
        n = left;
    memcpy(buf, data + cookie_in, n);
    cookie_in += n;
    return (ssize_t)n;
}

static ssize_t
cookie_write(void *cookie, const char *buf, size_t n)
{
    (void)cookie;
    if (n > sizeof(cookie_out) - cookie_out_len)
        n = sizeof(cookie_out) - cookie_out_len;
    memcpy(cookie_out + cookie_out_len, buf, n);
    cookie_out_len += n;
    return (ssize_t)n;
}

static int
cookie_close(void *cookie)
{
    (void)cookie;
    return 0;
}

static cookie_io_functions_t funcs = { cookie_read, cookie_write, 0, cookie_close };

static void
test_no_seek(void)
{
    char  buf[8];
    FILE *f = fopencookie(NULL, "r", funcs);

    if (!f) {
        fail("no-seek: setup");
        return;
    }
    setvbuf(f, buf, _IOFBF, sizeof(buf));
    if (fgetc(f) != 'A') {
        fail("no-seek: first char");
        fclose(f);
        return;
    }
    if (fflush(f) != 0)
        fail("no-seek: fflush must succeed without a seek function");
    if (fgetc(f) != 'B')
        fail("no-seek: buffered input lost");
    fclose(f);
}

#ifdef __PICOLIBC__

static void
test_direction_change(void)
{
    char  buf[8];
    FILE *f;

    cookie_in = 0;
    cookie_out_len = 0;
    f = fopencookie(NULL, "r+", funcs);
    if (!f) {
        fail("direction: setup");
        return;
    }
    setvbuf(f, buf, _IOFBF, sizeof(buf));
    if (fgetc(f) != 'A') {
        fail("direction: first char");
        fclose(f);
        return;
    }
    /*
     * In POSIX, write after read requires an intervening fflush or
     * positioning operation. We want to test picolibc behavior in
     * this case, so this test is restricted to picolibc.
     */
    if (fputc('X', f) != 'X')
        fail("direction: write after read must work");
    fclose(f);
    if (cookie_out_len != 1 || cookie_out[0] != 'X')
        fail("direction: buffered input written back as output");
}
#endif

int
main(void)
{
    test_read_ahead_kept();
    test_pushback_kept();
    test_close_partial();
    test_fseek_still_fails();
    test_no_seek();
#ifdef __PICOLIBC__
    test_direction_change();
#endif
    if (errors)
        printf("%d errors\n", errors);
    return errors != 0;
}
