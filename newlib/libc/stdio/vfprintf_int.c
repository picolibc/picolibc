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
    if (c == 'd' || c == 'i') {
        ultoa_signed_t x_s;

        arg_to_signed(ap, flags, x_s);

        if (x_s < 0) {
            /* Use unsigned in case x_s is the largest negative value */
            x_s = (ultoa_signed_t) -(ultoa_unsigned_t) x_s;
            flags |= FL_NEGATIVE;
        }

        flags &= ~FL_ALT;

#ifndef _NEED_IO_SHRINK
        if (x_s == 0 && (flags & FL_PREC) && prec == 0)
            buf_len = 0;
        else
#endif
            buf_len = __ultoa_invert (x_s, u.buf, 10) - u.buf;
    } else {
        int base;
        ultoa_unsigned_t x;

        if (c == 'u') {
            flags &= ~FL_ALT;
            base = 10;
        } else if (c == 'o') {
            base = 8;
            c = '\0';
        } else if (c == 'p') {
            base = 16;
            flags |= FL_ALT;
            c = 'x';
            if (sizeof(void *) > sizeof(int))
                flags |= FL_LONG;
        } else if (TOLOWER(c) == 'x') {
            base = ('x' - c) | 16;
#ifdef _NEED_IO_PERCENT_B
        } else if (TOLOWER(c) == 'b') {
            base = 2;
#endif
        } else {
            my_putc('%', stream);
            my_putc(c, stream);
            continue;
        }

        flags &= ~(FL_PLUS | FL_SPACE);

        arg_to_unsigned(ap, flags, x);

        /* Zero gets no special alternate treatment */
        if (x == 0)
            flags &= ~FL_ALT;

#ifndef _NEED_IO_SHRINK
        if (x == 0 && (flags & FL_PREC) && prec == 0)
            buf_len = 0;
        else
#endif
            buf_len = __ultoa_invert (x, u.buf, base) - u.buf;
    }

#ifndef _NEED_IO_SHRINK
    int len = buf_len;

    /* Specified precision */
    if (flags & FL_PREC) {

        /* Zfill ignored when precision specified */
        flags &= ~FL_ZFILL;

        /* If the number is shorter than the precision, pad
         * on the left with zeros */
        if (len < prec) {
            len = prec;

            /* Don't add the leading '0' for alternate octal mode */
            if (c == '\0')
                flags &= ~FL_ALT;
        }
    }

    /* Alternate mode for octal/hex */
    if (flags & FL_ALT) {

        len += 1;
        if (c != '\0')
            len += 1;
    } else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
        len += 1;
    }

    /* Pad on the left ? */
    if (!(flags & FL_LPAD)) {

        /* Pad with zeros, using the same loop as the
         * precision modifier
         */
        if (flags & FL_ZFILL) {
            prec = buf_len;
            if (len < width) {
                prec += width - len;
                len = width;
            }
        }
        while (len < width) {
            my_putc (' ', stream);
            len++;
        }
    }

    /* Width remaining on right after value */
    width -= len;

    /* Output leading characters */
    if (flags & FL_ALT) {
        my_putc ('0', stream);
        if (c != '\0')
            my_putc (c, stream);
    } else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
        unsigned char z = ' ';
        if (flags & FL_PLUS) z = '+';
        if (flags & FL_NEGATIVE) z = '-';
        my_putc (z, stream);
    }

    /* Output leading zeros */
    while (prec > buf_len) {
        my_putc ('0', stream);
        prec--;
    }
#else
    if (flags & FL_ALT) {
        my_putc ('0', stream);
        if (c != '\0')
            my_putc (c, stream);
    } else if (flags & FL_NEGATIVE)
        my_putc('-', stream);
#endif

    /* Output value */
    while (buf_len)
        my_putc (u.buf[--buf_len], stream);
}
