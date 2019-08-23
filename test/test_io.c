#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef TINY_STDIO
/* Buffered output routines for newlib-nano stdio */

static char write_buf[512];
static int write_len;

static int
ao_flush(FILE *ignore)
{
	if (write_len) {
		int off = 0;
		while (write_len) {
			int this = write(1, write_buf + off, write_len);
			if (this < 0) {
				write_len = 0;
				return -1;
			}
			off += this;
			write_len -= this;
		}
	}
	return 0;
}

static int
ao_putc(char c, FILE *ignore)
{
	write_buf[write_len++] = c;
	if (write_len < sizeof (write_buf))
		return 0;
	return ao_flush(ignore);
}

static char read_buf[512];
static int read_len;
static int read_off;

static int
ao_getc(FILE *ignore)
{
	if (read_off >= read_len) {
		(void) ao_flush(ignore);
		read_off = 0;
		read_len = read(0, read_buf, sizeof (read_buf));
		if (read_len <= 0) {
			read_len = 0;
			return -1;
		}
	}
	return (unsigned char) read_buf[read_off++];
}

static FILE __stdio = {
	.flags = __SRD|__SWR,
	.put = ao_putc,
	.get = ao_getc,
	.flush = ao_flush,
};

FILE *const __iob[3] = { &__stdio, &__stdio, &__stdio };

#else
int fstat(int fd, struct stat *statb) { errno = ENOTTY; return -1; }
#endif
