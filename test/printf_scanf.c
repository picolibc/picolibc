/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#ifndef TINY_STDIO
#define printf_float(x) ((double) (x))

#ifdef _NANO_FORMATTED_IO
#ifndef NO_FLOATING_POINT
extern int _printf_float();
extern int _scanf_float();

int (*_reference_printf_float)() = _printf_float;
int (*_reference_scanf_float)() = _scanf_float;
#endif
#endif
#endif

#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
static const double test_vals[] = { 1.234567, 1.1, M_PI };
#endif

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
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
	sprintf(buf, "%g", printf_float(0.0f));
	if (strcmp(buf, "0") != 0) {
		printf("0: wanted \"0\" got \"%s\"\n", buf);
		errors++;
		fflush(stdout);
	}
#endif
	for (x = 0; x < 32; x++) {
		unsigned int v = 0x12345678 >> x;
		unsigned int r;

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

#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
	for (x = -37; x <= 37; x++)
	{
		int t;
		for (t = 0; t < sizeof(test_vals)/sizeof(test_vals[0]); t++) {
#ifdef PICOLIBC_FLOAT_PRINTF_SCANF
#define float_type float
#define pow(a,b) powf((float) a, (float) b)
#define fabs(a) fabsf(a)
#define scanf_format "%f"
#if defined(TINY_STDIO) && !defined(_IO_FLOAT_EXACT)
#define ERROR_MAX 1e-6
#else
#define ERROR_MAX 0
#endif
#else
#define float_type double
#define scanf_format "%lf"
#if defined(TINY_STDIO) && !defined(_IO_FLOAT_EXACT)
#define ERROR_MAX 1e-15
#else
#define ERROR_MAX 0
#endif
#endif

			float_type v = (float_type) test_vals[t] * pow(10.0, (float_type) x);
			float_type r;
			float_type e;

			sprintf(buf, "%.55f", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX) {
				printf("\tf %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}


			sprintf(buf, "%.20e", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX)
			{
				printf("\te %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}


			sprintf(buf, "%.20g", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX)
			{
				printf("\tg %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}
		}
	}
#endif
	fflush(stdout);
	exit(errors);
}
