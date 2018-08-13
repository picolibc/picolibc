#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

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

/* Make sure the desired floating point conversion functions are linked in */
void *x = vfprintf;
void *y = vfscanf;

int
main(int argc, char **argv)
{
	int x = -35;
	char	buf[256];
	int	errors = 0;

	for (x = 0; x < 32; x++) {
		uint32_t v = 0x12345678ul >> x;
		uint32_t r;

		sprintf(buf, "%u", v);
		sscanf(buf, "%u", &r);
		if (v != r) {
			printf("\t%3d: wanted %u got %u\n", x, v, r);
			errors++;
			fflush(stdout);
		}
		sprintf(buf, "%x", v);
		sscanf(buf, "%x", &r);
		if (v != r) {
			printf("\t%3d: wanted %u got %u\n", x, v, r);
			errors++;
			fflush(stdout);
		}
		sprintf(buf, "%o", v);
		sscanf(buf, "%o", &r);
		if (v != r) {
			printf("\t%3d: wanted %u got %u\n", x, v, r);
			errors++;
			fflush(stdout);
		}
	}

	for (x = -37; x <= 37; x = x + 1)
	{
		float v = 1.23456f * powf(10.0f, (float) x);
		float r;
		float e;

		sprintf(buf, "%.45f", v);
		sscanf(buf, "%f", &r);
		e = fabsf(v-r) / v;
		if (e > 1e-6) {
			printf("\t%3d: wanted %.7e got %.7e (error %.7e\n", x, v, r, e);
			errors++;
			fflush(stdout);
		}

		sprintf(buf, "%.7e", v);
		sscanf(buf, "%f", &r);
		e = fabsf(v-r) / v;
		if (e > 1e-6) {
			printf("\t%3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x, v, r, e, buf);
			errors++;
			fflush(stdout);
		}
	}
	fflush(stdout);
	return errors;
}
