/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#define _GNU_SOURCE
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include "posix-local.h"

/*
 * Shared function for strfmon and strfmon_l
 *
 * Handles C/POSIX locale, which means:
 *
 * int_curr_symbol      ""
 * currency_symbol      ""
 * mon_decimal_point    ""
 * mon_thousands_sep    ""
 * mon_grouping         -1
 * positive_sign        ""
 * negative_sign        ""
 * int_frac_digits      -1
 * frac_digits          -1
 * p_cs_precedes        -1
 * p_sep_by_space       -1
 * n_cs_precedes        -1
 * n_sep_by_space       -1
 * p_sign_posn          -1
 * n_sign_posn          -1
 * int_p_cs_precedes    -1
 * int_p_sep_by_space   -1
 * int_n_cs_precedes    -1
 * int_n_sep_by_space   -1
 * int_p_sign_posn      -1
 * int_n_sign_posn      -1
 */

#ifdef __TINY_STDIO
# include "stdio_private.h"
# define m_snprintf	__d_snprintf	/* char equivalent function name */
#else
# define m_snprintf	snprintf	/* char equivalent function name */
#endif

ssize_t
__vstrfmon(char *__restrict buf, size_t size,
           const char *__restrict format, va_list ap)
{
    size_t      pos = 0;

#define output(c)       do {                    \
        if (pos == size) {                      \
            errno = E2BIG;                      \
            return -1;                          \
        }                                       \
        buf[pos++] = (c);                       \
    } while (0)

#define decimal(v) do {                         \
        (v) = c - '0';                          \
        for (;;) {                              \
            c = *format++;                      \
            if (!isdigit(c))                    \
                break;                          \
            (v) = (v) * 10 + c - '0';           \
        }                                       \
    } while (0)

    for (;;) {
        double val;
        char *val_pos;
        char c;
        char left_fill = ' ';
        bool sign_paren = false;
        bool left_justify = false;
        int field_width = -1;
        int left_precision = -1;
        int right_precision = -1;
        int val_width;
        int left_width;
        int total_width;
        size_t val_size;
        bool negative;

        /* Find the % */
        for (;;) {
            c = *format++;
            if (c == '%')
                break;
            if (c == '\0') {
                output('\0');
                return pos - 1;
            }
            output(c);
        }

        /* Parse flags */
        for (;;) {
            c = *format++;
            switch (c) {
            case '=':
                left_fill = *format++;
                if (!left_fill) {
                    errno = EINVAL;
                    return -1;
                }
                continue;
            case '^':
                continue;
            case '(':
                sign_paren = true;
                continue;
            case '+':
                /* Should this return -1/EINVAL? Glibc doesn't */
                sign_paren = false;
                continue;
            case '!':
                continue;
            case '-':
                left_justify = true;
                continue;
            }
            break;
        }

        /* parse field width */
        if (isdigit(c))
            decimal(field_width);

        /* parse left precision */
        if (c == '#') {
            c = *format++;
            if (!isdigit(c)) {
                errno = EINVAL;
                return -1;
            }
            decimal(left_precision);
        }

        /* parse right precision */
        if (c == '.') {
            c = *format++;
            if (!isdigit(c)) {
                errno = EINVAL;
                return -1;
            }
            decimal(right_precision);
        }

        /* parse conversion specifier char */
        switch (c) {
        case 'i':
        case 'n':
            val = va_arg(ap, double);
            negative = signbit(val) != 0;
            if (negative)
                val =-val;

            /* Write the value into the provided buffer */
            val_pos = buf + pos;
            val_size = size - pos;
            val_width = m_snprintf(val_pos, val_size, "%.*f", right_precision, val);

            /* Check to see if it fit */
            if (val_width < 0 || (size_t) val_width > val_size) {
                errno = E2BIG;
                return -1;
            }

            /* Trim trailing zeros */
            while (val_pos[val_width-1] == '0')
                val_width--;

            if (val_pos[val_width-1] == '.')
                val_width--;

            /* Compute width of integer portion (left) */
            left_width = strchrnul(val_pos, '.') - val_pos;

            /* Move it to the end of the buffer */
            memmove(buf + size - val_width, val_pos, val_width);
            val_pos = buf + size - val_width;

            /*
             * <field padding with spaces>
             * <sign character>
             * <left padding with left_fill>
             * <value>
             * <right paren if negative and sign_paren>
             * <field padding with spaces>
             */

            /* Compute total width of value */
            total_width = val_width;

            /* Add sign character */
            if (negative) {
                if (sign_paren)
                    total_width += 2;
                else
                    total_width += 1;
            } else if (left_precision >= 0)
                total_width += 1;

            /* Add left precision pad */
            if (left_precision > left_width)
                total_width += (left_precision - left_width);

            /* Generate the result */

            /* Pad field on left */
            if (!left_justify) {
                while (total_width < field_width) {
                    output(' ');
                    total_width++;
                }
            }

            /* Output sign character */
            if (negative) {
                if (sign_paren)
                    output('(');
                else
                    output('-');
            }
            else if (left_precision >= 0) {
                output(' ');
            }

            /* Pad value with left_fill */
            while (left_width < left_precision) {
                output(left_fill);
                left_width++;
            }

            /* Move val back into position */
            if (pos + val_width > size) {
                errno = E2BIG;
                return -1;
            }
            memmove(buf + pos, val_pos, val_width);
            pos += val_width;

            /* Output close paren for negative values */
            if (sign_paren) {
                if (negative)
                    output(')');
            }

            /* Pad field on right */
            if (left_justify) {
                while (total_width < field_width) {
                    output(' ');
                    total_width++;
                }
            }

            break;
        case '%':
            output(c);
            break;
        default:
            errno = EINVAL;
            return -1;
        }
    }
}
