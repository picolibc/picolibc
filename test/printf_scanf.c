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

extern double strtod(char *, char **);

static const double test_vals[] = { 1.234567, 1.1, M_PI };

int
main(int argc, char **argv)
{
	int x = -35;
	char	buf[256];
	int	errors = 0;
#if 0
	double	a;

	printf ("hello world\n");
	for (x = 1; x < 20; x++) {
		printf("%.*f\n", x, 9.99999999999);
	}

	for (a = 1e-10; a < 1e10; a *= 10.0) {
		printf("g format: %10.3g %10.3g\n", 1.2345678 * a, 1.1 * a);
		fflush(stdout);
	}
	for (a = 1e-10; a < 1e10; a *= 10.0) {
		printf("f format: %10.3f %10.3f\n", 1.2345678 * a, 1.1 * a);
		fflush(stdout);
	}
	for (a = 1e-10; a < 1e10; a *= 10.0) {
		printf("e format: %10.3e %10.3e\n", 1.2345678 * a, 1.1 * a);
		fflush(stdout);
	}
	printf ("%g\n", exp(11));
#endif
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

	for (x = -37; x <= 37; x++)
	{
		int t;
		for (t = 0; t < sizeof(test_vals)/sizeof(test_vals[0]); t++) {
			double v = test_vals[t] * pow(10.0, (double) x);
			double r;
			double e;

			sprintf(buf, "%.45f", v);
			sscanf(buf, "%lf", &r);
			e = fabs(v-r) / v;
			if (e > 1e-6) {
				printf("\t%3d: wanted %.7e got %.7e (error %.7e\n", x, v, r, e);
				errors++;
				fflush(stdout);
			}


			sprintf(buf, "%.14e", v);
			sscanf(buf, "%lf", &r);
			e = fabs(v-r) / v;
			if (e > 1e-6) {
				printf("\t%3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x, v, r, e, buf);
				errors++;
				fflush(stdout);
			}


			sprintf(buf, "%.7g", v);
			sscanf(buf, "%lf", &r);
			e = fabs(v-r) / v;
			if (e > 1e-6) {
				printf("\t%3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x, v, r, e, buf);
				errors++;
				fflush(stdout);
			}
		}
	}
	if (!errors)
		printf("success\n");
	fflush(stdout);
	return errors;
}
