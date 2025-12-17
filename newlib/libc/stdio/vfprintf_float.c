/* Copyright (c) 2002, Alexander Popov (sasho@vip.bg)
   Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2005, Helmut Wallner
   Copyright (c) 2007, Dmitry Xmelkov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
{
    uint8_t       sign;         /* sign character (or 0)	*/
    uint8_t       ndigs;        /* number of digits to convert */
    unsigned char case_convert; /* subtract to correct case */
    int           exp;          /* exponent of most significant decimal digit */
    int           n;            /* total width */
    uint8_t       ndigs_exp;    /* number of digis in exponent */

    /* deal with upper vs lower case */
    case_convert = TOLOWER(c) - c;
    c = TOLOWER(c);

#ifdef _NEED_IO_LONG_DOUBLE
    if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE)) {
        PRINTF_LONG_DOUBLE_TYPE fval;
        fval = PRINTF_LONG_DOUBLE_ARG(ap);

        ndigs = 0;

#ifdef _NEED_IO_C99_FORMATS
        if (c == 'a') {

            c = 'p';
            flags |= FL_FLTEXP | FL_FLTHEX;

            if (!(flags & FL_PREC))
                prec = -1;
            prec = __lfloat_x_engine(fval, &u.dtoa, prec, case_convert);
            ndigs = prec + 1;
            exp = u.dtoa.exp;
            ndigs_exp = 1;
        } else
#endif /* _NEED_IO_C99_FORMATS */
        {
            int  ndecimal = 0; /* digits after decimal (for 'f' format) */
            bool fmode = false;

            if (!(flags & FL_PREC))
                prec = 6;
            if (c == 'e') {
                ndigs = prec + 1;
                flags |= FL_FLTEXP;
            } else if (c == 'f') {
                ndigs = LONG_FLOAT_MAX_DIG;
                ndecimal = prec;
                flags |= FL_FLTFIX;
                fmode = true;
            } else {
                c += 'e' - 'g';
                ndigs = prec;
                if (ndigs < 1)
                    ndigs = 1;
            }

            if (ndigs > LONG_FLOAT_MAX_DIG)
                ndigs = LONG_FLOAT_MAX_DIG;

            ndigs = __lfloat_d_engine(fval, &u.dtoa, ndigs, fmode, ndecimal);

            exp = u.dtoa.exp;
            ndigs_exp = 2;
        }
    } else
#endif
    {
        FLOAT_UINT fval; /* value to print */
        fval = PRINTF_FLOAT_ARG(ap);

        ndigs = 0;

#ifdef _NEED_IO_C99_FORMATS
        if (c == 'a') {

            c = 'p';
            flags |= FL_FLTEXP | FL_FLTHEX;

            if (!(flags & FL_PREC))
                prec = -1;

            ndigs = 1 + __float_x_engine(fval, &u.dtoa, prec, case_convert);
            if (prec <= ndigs)
                prec = ndigs - 1;
            exp = u.dtoa.exp;
            ndigs_exp = 1;
        } else
#endif /* _NEED_IO_C99_FORMATS */
        {
            int  ndecimal = 0; /* digits after decimal (for 'f' format) */
            bool fmode = false;

            if (!(flags & FL_PREC))
                prec = 6;
            if (c == 'e') {
                ndigs = prec + 1;
                flags |= FL_FLTEXP;
            } else if (c == 'f') {
                ndigs = FLOAT_MAX_DIG;
                ndecimal = prec;
                flags |= FL_FLTFIX;
                fmode = true;
            } else {
                c += 'e' - 'g';
                ndigs = prec;
                if (ndigs < 1)
                    ndigs = 1;
            }

            if (ndigs > FLOAT_MAX_DIG)
                ndigs = FLOAT_MAX_DIG;

            ndigs = __float_d_engine(fval, &u.dtoa, ndigs, fmode, ndecimal);
            exp = u.dtoa.exp;
            ndigs_exp = 2;
        }
    }
    if (exp < -9 || 9 < exp)
        ndigs_exp = 2;
    if (exp < -99 || 99 < exp)
        ndigs_exp = 3;
#ifdef _NEED_IO_FLOAT64
    if (exp < -999 || 999 < exp)
        ndigs_exp = 4;
#ifdef _NEED_IO_FLOAT_LARGE
    if (exp < -9999 || 9999 < exp)
        ndigs_exp = 5;
#endif
#endif

    sign = 0;
    if (u.dtoa.flags & DTOA_MINUS)
        sign = '-';
    else if (flags & FL_PLUS)
        sign = '+';
    else if (flags & FL_SPACE)
        sign = ' ';

    if (u.dtoa.flags & (DTOA_NAN | DTOA_INF)) {
        ndigs = sign ? 4 : 3;
        if (width > ndigs) {
            width -= ndigs;
            if (!(flags & FL_LPAD)) {
                do {
                    my_putc(' ', stream);
                } while (--width);
            }
        } else {
            width = 0;
        }
        if (sign)
            my_putc(sign, stream);
        pnt = "inf";
        if (u.dtoa.flags & DTOA_NAN)
            pnt = "nan";
        while ((c = *pnt++))
            my_putc(TOCASE(c), stream);
    } else {

        if (!(flags & (FL_FLTEXP | FL_FLTFIX))) {

            /* 'g(G)' format */

            /*
             * On entry to this block, prec is
             * the number of digits to display.
             *
             * On exit, prec is the number of digits
             * to display after the decimal point
             */

            /* Always show at least one digit */
            if (prec == 0)
                prec = 1;

            /*
             * Remove trailing zeros. The ryu code can emit them
             * when rounding to fewer digits than required for
             * exact output, the imprecise code often emits them
             */
            while (ndigs > 0 && u.dtoa.digits[ndigs - 1] == '0')
                ndigs--;

            /* Save requested precision */
            int req_prec = prec;

            /* Limit output precision to ndigs unless '#' */
            if (!(flags & FL_ALT))
                prec = ndigs;

            /*
             * Figure out whether to use 'f' or 'e' format. The spec
             * says to use 'f' if the exponent is >= -4 and < requested
             * precision.
             */
            if (-4 <= exp && exp < req_prec) {
                flags |= FL_FLTFIX;

                /* Compute how many digits to show after the decimal.
                 *
                 * If exp is negative, then we need to show that
                 * many leading zeros plus the requested precision
                 *
                 * If exp is less than prec, then we need to show a
                 * number of digits past the decimal point,
                 * including (potentially) some trailing zeros
                 *
                 * (these two cases end up computing the same value,
                 * and are both caught by the exp < prec test,
                 * so they share the same branch of the 'if')
                 *
                 * If exp is at least 'prec', then we don't show
                 * any digits past the decimal point.
                 */
                if (exp < prec)
                    prec = prec - (exp + 1);
                else
                    prec = 0;
            } else {
                /* Compute how many digits to show after the decimal */
                prec = prec - 1;
            }
        }

        /* Conversion result length, width := free space length	*/
        if (flags & FL_FLTFIX)
            n = (exp > 0 ? exp + 1 : 1);
        else {
            n = 3; /* 1e+ */
#ifdef _NEED_IO_C99_FORMATS
            if (flags & FL_FLTHEX)
                n += 2; /* or 0x1p+ */
#endif
            n += ndigs_exp; /* add exponent */
        }
        if (sign)
            n += 1;
        if (prec)
            n += prec + 1;
        else if (flags & FL_ALT)
            n += 1;

        width = width > n ? width - n : 0;

        /* Output before first digit	*/
        if (!(flags & (FL_LPAD | FL_ZFILL))) {
            while (width) {
                my_putc(' ', stream);
                width--;
            }
        }
        if (sign)
            my_putc(sign, stream);

#ifdef _NEED_IO_C99_FORMATS
        if ((flags & FL_FLTHEX)) {
            my_putc('0', stream);
            my_putc(TOCASE('x'), stream);
        }
#endif

        if (!(flags & FL_LPAD)) {
            while (width) {
                my_putc('0', stream);
                width--;
            }
        }

        if (flags & FL_FLTFIX) { /* 'f' format		*/
            char out;

            /* At this point, we should have
             *
             *	exp	exponent of leftmost digit in u.dtoa.digits
             *	ndigs	number of buffer digits to print
             *	prec	number of digits after decimal
             *
             * In the loop, 'n' walks over the exponent value
             */
            n = exp > 0 ? exp : 0; /* exponent of left digit */
            do {

                /* Insert decimal point at correct place */
                if (n == -1)
                    my_putc('.', stream);

                /* Pull digits from buffer when in-range,
                 * otherwise use 0
                 */
                if (0 <= exp - n && exp - n < ndigs)
                    out = u.dtoa.digits[exp - n];
                else
                    out = '0';
                if (--n < -prec) {
                    break;
                }
                my_putc(out, stream);
            } while (1);
            my_putc(out, stream);
            if ((flags & FL_ALT) && n == -1)
                my_putc('.', stream);
        } else { /* 'e(E)' format	*/

            /* mantissa	*/
            my_putc(u.dtoa.digits[0], stream);
            if (prec > 0) {
                my_putc('.', stream);
                int pos = 1;
                for (pos = 1; pos < 1 + prec; pos++)
                    my_putc(pos < ndigs ? u.dtoa.digits[pos] : '0', stream);
            } else if (flags & FL_ALT)
                my_putc('.', stream);

            /* exponent	*/
            my_putc(TOCASE(c), stream);
            sign = '+';
            if (exp < 0) {
                exp = -exp;
                sign = '-';
            }
            my_putc(sign, stream);
#ifdef _NEED_IO_FLOAT_LARGE
            if (ndigs_exp > 4) {
                my_putc(exp / 10000 + '0', stream);
                exp %= 10000;
            }
#endif
#ifdef _NEED_IO_FLOAT64
            if (ndigs_exp > 3) {
                my_putc(exp / 1000 + '0', stream);
                exp %= 1000;
            }
#endif
            if (ndigs_exp > 2) {
                my_putc(exp / 100 + '0', stream);
                exp %= 100;
            }
            if (ndigs_exp > 1) {
                my_putc(exp / 10 + '0', stream);
                exp %= 10;
            }
            my_putc('0' + exp, stream);
        }
    }
}
