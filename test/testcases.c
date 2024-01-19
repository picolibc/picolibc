/*
 * This file was generated from printf-tests.txt included
 * as a part of the printf test suite developed by
 * Bart Massey:
 *
 * https://github.com/BartMassey/printf-tests
 *
 * printf-tests.txt carries the following Copyright, which
 * probably extends to this file as well given the transformations
 * are fairly mechanical:
 *
# Various printf tests compiled from various sources
# Copyright © 2013 Bart Massey
# This program is licensed under the GPL version 2 or later.
# Please see the file COPYING.GPL2 in this distribution for
# license terms.

*/

#ifndef __PICOLIBC__
# define _WANT_IO_C99_FORMATS
# define _WANT_IO_LONG_LONG
# define _WANT_IO_POS_ARGS
#elif defined(TINY_STDIO)
# ifdef _HAS_IO_PERCENT_B
#  define BINARY_FORMAT
# endif
# ifdef _HAS_IO_DOUBLE
#  if __SIZEOF_DOUBLE__ == 4
#   define LOW_FLOAT
#  endif
# elif defined(_HAS_IO_FLOAT)
#  define LOW_FLOAT
# else
#  define NO_FLOAT
# endif
# ifndef _HAS_IO_LONG_LONG
#  define NO_LONGLONG
# endif
# ifndef _HAS_IO_POS_ARGS
#  define NO_POS_ARGS
# endif
# ifdef PICOLIBC_MINIMAL_PRINTF_SCANF
#  define NO_WIDTH_PREC
#  define NO_CASE_HEX
# endif
#else
# if __SIZEOF_DOUBLE__ == 4
#  define LOW_FLOAT
# endif
# ifdef NO_FLOATING_POINT
#  define NO_FLOAT
# endif
# ifndef _WANT_IO_LONG_LONG
#  define NO_LONGLONG
# endif
# ifndef _WANT_IO_POS_ARGS
#  define NO_POS_ARGS
# endif
# define NORMALIZED_A
#endif


#if __SIZEOF_INT__ < 4
#define I(a,b) (b)
#else
#define I(a,b) (a)
#endif

//{
/* XXX This code generated automatically by gen-testcases.hs
   from ../../printf-tests.txt . You probably do not want to
   manually edit this file. */
#ifndef NO_FLOAT
    result |= test(__LINE__, "0", "%.7g", 0.0);
    result |= test(__LINE__, "0.33", "%.*f", 2, 0.33333333);
#endif
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "foo", "%.3s", "foobar");
    result |= test(__LINE__, "     00004", "%10.5d", 4);
    result |= test(__LINE__, " 42", "% d", 42);
#endif
    result |= test(__LINE__, "-42", "% d", -42);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "   42", "% 5d", 42);
    result |= test(__LINE__, "  -42", "% 5d", -42);
    result |= test(__LINE__, "             42", "% 15d", 42);
    result |= test(__LINE__, "            -42", "% 15d", -42);
    result |= test(__LINE__, "+42", "%+d", 42);
    result |= test(__LINE__, "-42", "%+d", -42);
    result |= test(__LINE__, "  +42", "%+5d", 42);
    result |= test(__LINE__, "  -42", "%+5d", -42);
    result |= test(__LINE__, "            +42", "%+15d", 42);
    result |= test(__LINE__, "            -42", "%+15d", -42);
#endif
    result |= test(__LINE__, "42", "%0d", 42);
    result |= test(__LINE__, "-42", "%0d", -42);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "00042", "%05d", 42);
    result |= test(__LINE__, "-0042", "%05d", -42);
    result |= test(__LINE__, "000000000000042", "%015d", 42);
    result |= test(__LINE__, "-00000000000042", "%015d", -42);
#endif
    result |= test(__LINE__, "42", "%-d", 42);
    result |= test(__LINE__, "-42", "%-d", -42);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "42   ", "%-5d", 42);
    result |= test(__LINE__, "-42  ", "%-5d", -42);
    result |= test(__LINE__, "42             ", "%-15d", 42);
    result |= test(__LINE__, "-42            ", "%-15d", -42);
#endif
    result |= test(__LINE__, "42", "%-0d", 42);
    result |= test(__LINE__, "-42", "%-0d", -42);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "42   ", "%-05d", 42);
    result |= test(__LINE__, "-42  ", "%-05d", -42);
    result |= test(__LINE__, "42             ", "%-015d", 42);
    result |= test(__LINE__, "-42            ", "%-015d", -42);
#endif
    result |= test(__LINE__, "42", "%0-d", 42);
    result |= test(__LINE__, "-42", "%0-d", -42);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "42   ", "%0-5d", 42);
    result |= test(__LINE__, "-42  ", "%0-5d", -42);
    result |= test(__LINE__, "42             ", "%0-15d", 42);
    result |= test(__LINE__, "-42            ", "%0-15d", -42);
#endif
#ifndef NO_FLOAT
    result |= test(__LINE__, "42.90", "%.2f", 42.8952);
    result |= test(__LINE__, "42.90", "%.2F", 42.8952);
#ifdef LOW_FLOAT
    result |= test(__LINE__, "42.89520", "%.5f", 42.8952);
#else
    result |= test(__LINE__, "42.8952000000", "%.10f", 42.8952);
#endif
    result |= test(__LINE__, "42.90", "%1.2f", 42.8952);
    result |= test(__LINE__, " 42.90", "%6.2f", 42.8952);
    result |= test(__LINE__, "+42.90", "%+6.2f", 42.8952);
#ifdef LOW_FLOAT
    result |= test(__LINE__, "42.89520", "%5.5f", 42.8952);
#else
    result |= test(__LINE__, "42.8952000000", "%5.10f", 42.8952);
#endif
#endif /* NO_FLOAT */
    /* 51: anti-test */
    /* 52: anti-test */
    /* 53: excluded for C */
#if !defined(NO_POS_ARGS)
    result |= test(__LINE__, "Hot Pocket", "%1$s %2$s", "Hot", "Pocket");
    result |= test(__LINE__, "Pocket Hot", "%2$s %1$s", "Hot", "Pocket");
    result |= test(__LINE__, "0002   1 hi", "%2$04d %1$*3$d %4$s", 1, 2, 3, "hi");
    result |= test(__LINE__, "   ab", "%1$*2$.*3$s", "abc", 5, 2);
#ifndef NO_FLOAT
    result |= test(__LINE__, "12.0 Hot Pockets", "%1$.1f %2$s %3$ss", printf_float(12.0), "Hot", "Pocket");
    result |= test(__LINE__, "12.0 Hot Pockets", "%1$.*4$f %2$s %3$ss", printf_float(12.0), "Hot", "Pocket", 1);
    result |= test(__LINE__, " 12.0 Hot Pockets", "%1$*5$.*4$f %2$s %3$ss", printf_float(12.0), "Hot", "Pocket", 1, 5);
    result |= test(__LINE__, " 12.0 Hot Pockets 5", "%1$5.*4$f %2$s %3$ss %5$d", printf_float(12.0), "Hot", "Pocket", 1, 5);
#if !defined(TINY_STDIO) || defined(_IO_FLOAT_EXACT)
    result |= test(__LINE__,
                   "   12345  1234    11145401322     321.765400   3.217654e+02   5    test-string",
                   "%1$*5$d %2$*6$hi %3$*7$lo %4$*8$f %9$*12$e %10$*13$g %11$*14$s",
                   12345, 1234, 1234567890, printf_float(321.7654), 8, 5, 14, 14,
                   printf_float(321.7654), printf_float(5.0000001), "test-string", 14, 3, 14);
#endif
#endif
#endif
    /* 58: anti-test */
#ifdef TINY_STDIO
    result |= test(__LINE__, "%(foo", "%(foo");
#endif
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, " foo", "%*s", 4, "foo");
#endif
#ifndef NO_FLOAT
    result |= test(__LINE__, "      3.14", "%*.*f", 10, 2, 3.14159265);
    result |= test(__LINE__, "3.14      ", "%-*.*f", 10, 2, 3.14159265);
# if !(defined(TINY_STDIO) && !defined(_IO_FLOAT_EXACT))
#  ifndef LOW_FLOAT
#   ifdef TINY_STDIO
#    define SQRT2_60 "1414213562373095000000000000000000000000000000000000000000000.000"
#   else
#    define SQRT2_60 "1414213562373095053224405813183213153460812619236586568024064.000"
#   endif
    result |= test(__LINE__, SQRT2_60, "%.3f", 1.4142135623730950e60);
#  endif
# endif
#endif
    /* 64: anti-test */
    /* 65: anti-test */
    result |= test(__LINE__, "+hello+", "+%s+", "hello");
    result |= test(__LINE__, "+10+", "+%d+", 10);
    result |= test(__LINE__, "a", "%c", 'a');
    result |= test(__LINE__, " ", "%c", 32);
    result |= test(__LINE__, "$", "%c", 36);
    result |= test(__LINE__, "10", "%d", 10);

#ifndef _NANO_FORMATTED_IO
    result |= test(__LINE__, I("1000000","16960"), "%'d", 1000000);
#endif
    /* 72: anti-test */
    /* 73: anti-test */
    /* 74: excluded for C */
    /* 75: excluded for C */
#ifndef NO_FLOAT
#ifdef LOW_FLOAT
    result |= test(__LINE__, "         +7.894561e+08", "%+#22.6e", 7.89456123e8);
    result |= test(__LINE__, "7.894561e+08          ", "%-#22.6e", 7.89456123e8);
    result |= test(__LINE__, "          7.894561e+08", "%#22.6e", 7.89456123e8);
#else
    result |= test(__LINE__, "+7.894561230000000e+08", "%+#22.15e", 7.89456123e8);
    result |= test(__LINE__, "7.894561230000000e+08 ", "%-#22.15e", 7.89456123e8);
    result |= test(__LINE__, " 7.894561230000000e+08", "%#22.15e", 7.89456123e8);
#endif
    result |= test(__LINE__, "8.e+08", "%#1.1g", 7.89456123e8);
#endif
#ifndef NO_LONGLONG
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "    +100", "%+8lld", 100LL);
#if defined(TINY_STDIO) || !defined(__PICOLIBC__)
    result |= test(__LINE__, "    +100", "%+8Ld", 100LL);
#endif
    result |= test(__LINE__, "+00000100", "%+.8lld", 100LL);
    result |= test(__LINE__, " +00000100", "%+10.8lld", 100LL);
#endif
#ifdef TINY_STDIO
    result |= test(__LINE__, "%_1lld", "%_1lld", 100LL);
#endif
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "-00100", "%-1.5lld", -100LL);
    result |= test(__LINE__, "  100", "%5lld", 100LL);
    result |= test(__LINE__, " -100", "%5lld", -100LL);
    result |= test(__LINE__, "100  ", "%-5lld", 100LL);
    result |= test(__LINE__, "-100 ", "%-5lld", -100LL);
    result |= test(__LINE__, "00100", "%-.5lld", 100LL);
    result |= test(__LINE__, "-00100", "%-.5lld", -100LL);
    result |= test(__LINE__, "00100   ", "%-8.5lld", 100LL);
    result |= test(__LINE__, "-00100  ", "%-8.5lld", -100LL);
    result |= test(__LINE__, "00100", "%05lld", 100LL);
    result |= test(__LINE__, "-0100", "%05lld", -100LL);
    result |= test(__LINE__, " 100", "% lld", 100LL);
    result |= test(__LINE__, "-100", "% lld", -100LL);
    result |= test(__LINE__, "  100", "% 5lld", 100LL);
    result |= test(__LINE__, " -100", "% 5lld", -100LL);
    result |= test(__LINE__, " 00100", "% .5lld", 100LL);
    result |= test(__LINE__, "-00100", "% .5lld", -100LL);
    result |= test(__LINE__, "   00100", "% 8.5lld", 100LL);
    result |= test(__LINE__, "  -00100", "% 8.5lld", -100LL);
    result |= test(__LINE__, "", "%.0lld", 0LL);
    result |= test(__LINE__, " 0x00ffffffffffffff9c", "%#+21.18llx", -100LL);
    result |= test(__LINE__, "0001777777777777777777634", "%#.25llo", -100LL);
    result |= test(__LINE__, " 01777777777777777777634", "%#+24.20llo", -100LL);
    result |= test(__LINE__, "0X00000FFFFFFFFFFFFFF9C", "%#+18.21llX", -100LL);
    result |= test(__LINE__, "001777777777777777777634", "%#+20.24llo", -100LL);
    result |= test(__LINE__, "   0018446744073709551615", "%#+25.22llu", -1LL);
    result |= test(__LINE__, "   0018446744073709551615", "%#+25.22llu", -1LL);
    result |= test(__LINE__, "     0000018446744073709551615", "%#+30.25llu", -1LL);
    result |= test(__LINE__, "  -0000000000000000000001", "%+#25.22lld", -1LL);
    result |= test(__LINE__, "00144   ", "%#-8.5llo", 100LL);
    result |= test(__LINE__, "+00100  ", "%#-+ 08.5lld", 100LL);
    result |= test(__LINE__, "+00100  ", "%#-+ 08.5lld", 100LL);
    result |= test(__LINE__, "0000000000000000000000000000000000000001", "%.40lld", 1LL);
    result |= test(__LINE__, " 0000000000000000000000000000000000000001", "% .40lld", 1LL);
#endif
#endif
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, " 0000000000000000000000000000000000000001", "% .40d", 1);
#endif
    /* 121: excluded for C */
    /* 124: excluded for C */
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, " 1", "% d", 1);
    result |= test(__LINE__, "+1", "%+ d", 1);
    result |= test(__LINE__, "0x0000000001", "%#012x", 1);
    result |= test(__LINE__, "0x00000001", "%#04.8x", 1);
    result |= test(__LINE__, "0x01    ", "%#-08.2x", 1);
    result |= test(__LINE__, "00000001", "%#08o", 1);
#endif
    result |= test(__LINE__, "0x39", "%p", (void *)57ULL);
    result |= test(__LINE__, "0x39", "%p", (void *)57U);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "f", "%.1s", "foo");
    result |= test(__LINE__, "f", "%.*s", 1, "foo");
    result |= test(__LINE__, "foo  ", "%*s", -5, "foo");
#endif
    result |= test(__LINE__, "hello", "hello");
#if defined(TINY_STDIO) && !defined(_HAS_IO_PERCENT_B)
    result |= test(__LINE__, "%b", "%b");
#endif
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "  a", "%3c", 'a');
#endif
    result |= test(__LINE__, "1234", "%3d", 1234);
    /* 150: excluded for C */
    result |= test(__LINE__, "2", "%-1d", 2);
#ifndef NO_FLOAT
    result |= test(__LINE__, "8.6000", "%2.4f", 8.6);
    result |= test(__LINE__, "0.600000", "%0f", 0.6);
    result |= test(__LINE__, "1", "%.0f", 0.6);
    result |= test(__LINE__, "8.6000e+00", "%2.4e", 8.6);
    result |= test(__LINE__, " 8.6000e+00", "% 2.4e", 8.6);
    result |= test(__LINE__, "-8.6000e+00", "% 2.4e", -8.6);
    result |= test(__LINE__, "+8.6000e+00", "%+2.4e", 8.6);
    result |= test(__LINE__, "8.6", "%2.4g", 8.6);
#endif
    result |= test(__LINE__, "-1", "%-i", -1);
    result |= test(__LINE__, "1", "%-i", 1);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "+1", "%+i", 1);
#endif
    result |= test(__LINE__, "12", "%o", 10);
    /* 166: excluded for C */
    /* 167: excluded for C */
#ifdef TINY_STDIO
    result |= test(__LINE__, "(null)", "%s", NULL);
#endif
    result |= test(__LINE__, "%%%%", "%s", "%%%%");
    result |= test(__LINE__, I("4294967295", "65535"), "%u", -1);
#ifdef TINY_STDIO
    result |= test(__LINE__, "%w", "%w", -1);
#endif
    /* 172: excluded for C */
    /* 173: excluded for C */
    /* 174: excluded for C */
#ifdef TINY_STDIO
    result |= test(__LINE__, "%H", "%H", -1);
#endif
    result |= test(__LINE__, "%0", "%%0");
    result |= test(__LINE__, "2345", "%hx", 74565);
#ifndef _NANO_FORMATTED_IO
    result |= test(__LINE__, "61", "%hhx", 0x61);
    result |= test(__LINE__, "61", "%hhx", 0x161);
    result |= test(__LINE__, "97", "%hhd", 0x61);
    result |= test(__LINE__, "97", "%hhd", 0x161);
    result |= test(__LINE__, "-97", "%hhd", -0x61);
    result |= test(__LINE__, "-97", "%hhd", -0x161);
#endif
    result |= test(__LINE__, "Hallo heimur", "Hallo heimur");
    result |= test(__LINE__, "Hallo heimur", "%s", "Hallo heimur");
    result |= test(__LINE__, "1024", "%d", 1024);
    result |= test(__LINE__, "-1024", "%d", -1024);
    result |= test(__LINE__, "1024", "%i", 1024);
    result |= test(__LINE__, "-1024", "%i", -1024);
    result |= test(__LINE__, "1024", "%u", 1024);
    result |= test(__LINE__, I("4294966272", "64512"), "%u", 4294966272U);
    result |= test(__LINE__, "777", "%o", 511);
    result |= test(__LINE__, I("37777777001", "177001"), "%o", 4294966785U);
    result |= test(__LINE__, I("1234abcd", "abcd"), "%x", 305441741);
    result |= test(__LINE__, I("edcb5433", "5433"), "%x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD", "ABCD"), "%X", 305441741);
    result |= test(__LINE__, I("EDCB5433", "5433"), "%X", 3989525555U);
#ifdef BINARY_FORMAT
    result |= test(__LINE__, I("10010001101001010101111001101", "1010101111001101"), "%b", 305441741);
    result |= test(__LINE__, I("11101101110010110101010000110011", "101010000110011"), "%b", 3989525555U);
    result |= test(__LINE__, I("10010001101001010101111001101", "1010101111001101"), "%B", 305441741);
    result |= test(__LINE__, I("11101101110010110101010000110011", "101010000110011"), "%B", 3989525555U);
#endif
    result |= test(__LINE__, "x", "%c", 'x');
    result |= test(__LINE__, "%", "%%");
    result |= test(__LINE__, "Hallo heimur", "%+s", "Hallo heimur");
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "+1024", "%+d", 1024);
#endif
    result |= test(__LINE__, "-1024", "%+d", -1024);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "+1024", "%+i", 1024);
#endif
    result |= test(__LINE__, "-1024", "%+i", -1024);
    result |= test(__LINE__, "1024", "%+u", 1024);
    result |= test(__LINE__, I("4294966272", "64512"), "%+u", 4294966272U);
    result |= test(__LINE__, "777", "%+o", 511);
    result |= test(__LINE__, I("37777777001", "177001"), "%+o", 4294966785U);
    result |= test(__LINE__, I("1234abcd", "abcd"), "%+x", 305441741);
    result |= test(__LINE__, I("edcb5433", "5433"), "%+x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD", "ABCD"), "%+X", 305441741);
    result |= test(__LINE__, I("EDCB5433", "5433"), "%+X", 3989525555U);
    result |= test(__LINE__, "x", "%+c", 'x');
    result |= test(__LINE__, "Hallo heimur", "% s", "Hallo heimur");
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, " 1024", "% d", 1024);
#endif
    result |= test(__LINE__, "-1024", "% d", -1024);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, " 1024", "% i", 1024);
#endif
    result |= test(__LINE__, "-1024", "% i", -1024);
    result |= test(__LINE__, "1024", "% u", 1024);
    result |= test(__LINE__, I("4294966272", "64512"), "% u", 4294966272U);
    result |= test(__LINE__, "777", "% o", 511);
    result |= test(__LINE__, I("37777777001", "177001"), "% o", 4294966785U);
    result |= test(__LINE__, I("1234abcd", "abcd"), "% x", 305441741);
    result |= test(__LINE__, I("edcb5433", "5433"), "% x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD", "ABCD"), "% X", 305441741);
    result |= test(__LINE__, I("EDCB5433", "5433"), "% X", 3989525555U);
    result |= test(__LINE__, "x", "% c", 'x');
    result |= test(__LINE__, "Hallo heimur", "%+ s", "Hallo heimur");
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "+1024", "%+ d", 1024);
#endif
    result |= test(__LINE__, "-1024", "%+ d", -1024);
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "+1024", "%+ i", 1024);
#endif
    result |= test(__LINE__, "-1024", "%+ i", -1024);
    result |= test(__LINE__, "1024", "%+ u", 1024);
    result |= test(__LINE__, I("4294966272", "64512"), "%+ u", 4294966272U);
    result |= test(__LINE__, "777", "%+ o", 511);
    result |= test(__LINE__, I("37777777001", "177001"), "%+ o", 4294966785U);
    result |= test(__LINE__, I("1234abcd", "abcd"), "%+ x", 305441741);
    result |= test(__LINE__, I("edcb5433", "5433"), "%+ x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD", "ABCD"), "%+ X", 305441741);
    result |= test(__LINE__, I("EDCB5433", "5433"), "%+ X", 3989525555U);
    result |= test(__LINE__, "x", "%+ c", 'x');
    result |= test(__LINE__, "0777", "%#o", 511);
    result |= test(__LINE__, I("037777777001", "0177001"), "%#o", 4294966785U);
    result |= test(__LINE__, I("0x1234abcd", "0xabcd"), "%#x", 305441741);
    result |= test(__LINE__, I("0xedcb5433", "0x5433"), "%#x", 3989525555U);
#ifndef NO_CASE_HEX
    result |= test(__LINE__, I("0X1234ABCD", "0XABCD"), "%#X", 305441741);
    result |= test(__LINE__, I("0XEDCB5433", "0X5433"), "%#X", 3989525555U);
#endif
#ifdef BINARY_FORMAT
    result |= test(__LINE__, I("0b10010001101001010101111001101", "0b1010101111001101"), "%#b", 305441741);
    result |= test(__LINE__, I("0b11101101110010110101010000110011", "0b101010000110011"), "%#b", 3989525555U);
    result |= test(__LINE__, I("0B10010001101001010101111001101", "0B1010101111001101"), "%#B", 305441741);
    result |= test(__LINE__, I("0B11101101110010110101010000110011", "0B101010000110011"), "%#B", 3989525555U);
#endif
    result |= test(__LINE__, "0", "%#o", 0U);
    result |= test(__LINE__, "0", "%#x", 0U);
    result |= test(__LINE__, "0", "%#X", 0U);
    result |= test(__LINE__, "Hallo heimur", "%1s", "Hallo heimur");
    result |= test(__LINE__, "1024", "%1d", 1024);
    result |= test(__LINE__, "-1024", "%1d", -1024);
    result |= test(__LINE__, "1024", "%1i", 1024);
    result |= test(__LINE__, "-1024", "%1i", -1024);
    result |= test(__LINE__, "1024", "%1u", 1024);
    result |= test(__LINE__, I("4294966272", "64512"), "%1u", 4294966272U);
    result |= test(__LINE__, "777", "%1o", 511);
    result |= test(__LINE__, I("37777777001", "177001"), "%1o", 4294966785U);
    result |= test(__LINE__, I("1234abcd", "abcd"), "%1x", 305441741);
    result |= test(__LINE__, I("edcb5433", "5433"), "%1x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD", "ABCD"), "%1X", 305441741);
    result |= test(__LINE__, I("EDCB5433", "5433"), "%1X", 3989525555U);
    result |= test(__LINE__, "x", "%1c", 'x');
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "               Hallo", "%20s", "Hallo");
    result |= test(__LINE__, "                1024", "%20d", 1024);
    result |= test(__LINE__, "               -1024", "%20d", -1024);
    result |= test(__LINE__, "                1024", "%20i", 1024);
    result |= test(__LINE__, "               -1024", "%20i", -1024);
    result |= test(__LINE__, "                1024", "%20u", 1024);
    result |= test(__LINE__, I("          4294966272", "               64512"), "%20u", 4294966272U);
    result |= test(__LINE__, "                 777", "%20o", 511);
    result |= test(__LINE__, I("         37777777001", "              177001"), "%20o", 4294966785U);
    result |= test(__LINE__, I("            1234abcd", "                abcd"), "%20x", 305441741);
    result |= test(__LINE__, I("            edcb5433", "                5433"), "%20x", 3989525555U);
    result |= test(__LINE__, I("            1234ABCD", "                ABCD"), "%20X", 305441741);
    result |= test(__LINE__, I("            EDCB5433", "                5433"), "%20X", 3989525555U);
    result |= test(__LINE__, "                   x", "%20c", 'x');
    result |= test(__LINE__, "Hallo               ", "%-20s", "Hallo");
    result |= test(__LINE__, "1024                ", "%-20d", 1024);
    result |= test(__LINE__, "-1024               ", "%-20d", -1024);
    result |= test(__LINE__, "1024                ", "%-20i", 1024);
    result |= test(__LINE__, "-1024               ", "%-20i", -1024);
    result |= test(__LINE__, "1024                ", "%-20u", 1024);
    result |= test(__LINE__, I("4294966272          ", "64512               "), "%-20u", 4294966272U);
    result |= test(__LINE__, "777                 ", "%-20o", 511);
    result |= test(__LINE__, I("37777777001         ", "177001              "), "%-20o", 4294966785U);
    result |= test(__LINE__, I("1234abcd            ", "abcd                "), "%-20x", 305441741);
    result |= test(__LINE__, I("edcb5433            ", "5433                "), "%-20x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD            ", "ABCD                "), "%-20X", 305441741);
    result |= test(__LINE__, I("EDCB5433            ", "5433                "), "%-20X", 3989525555U);
    result |= test(__LINE__, "x                   ", "%-20c", 'x');
    result |= test(__LINE__, "00000000000000001024", "%020d", 1024);
    result |= test(__LINE__, "-0000000000000001024", "%020d", -1024);
    result |= test(__LINE__, "00000000000000001024", "%020i", 1024);
    result |= test(__LINE__, "-0000000000000001024", "%020i", -1024);
    result |= test(__LINE__, "00000000000000001024", "%020u", 1024);
    result |= test(__LINE__, I("00000000004294966272", "00000000000000064512"), "%020u", 4294966272U);
    result |= test(__LINE__, "00000000000000000777", "%020o", 511);
    result |= test(__LINE__, I("00000000037777777001", "00000000000000177001"), "%020o", 4294966785U);
    result |= test(__LINE__, I("0000000000001234abcd", "0000000000000000abcd"), "%020x", 305441741);
    result |= test(__LINE__, I("000000000000edcb5433", "00000000000000005433"), "%020x", 3989525555U);
    result |= test(__LINE__, I("0000000000001234ABCD", "0000000000000000ABCD"), "%020X", 305441741);
    result |= test(__LINE__, I("000000000000EDCB5433", "00000000000000005433"), "%020X", 3989525555U);
    result |= test(__LINE__, "                0777", "%#20o", 511);
    result |= test(__LINE__, I("        037777777001", "             0177001"), "%#20o", 4294966785U);
    result |= test(__LINE__, I("          0x1234abcd", "              0xabcd"), "%#20x", 305441741);
    result |= test(__LINE__, I("          0xedcb5433", "              0x5433"), "%#20x", 3989525555U);
    result |= test(__LINE__, I("          0X1234ABCD", "              0XABCD"), "%#20X", 305441741);
    result |= test(__LINE__, I("          0XEDCB5433", "              0X5433"), "%#20X", 3989525555U);
    result |= test(__LINE__, "00000000000000000777", "%#020o", 511);
    result |= test(__LINE__, I("00000000037777777001", "00000000000000177001"), "%#020o", 4294966785U);
    result |= test(__LINE__, I("0x00000000001234abcd", "0x00000000000000abcd"), "%#020x", 305441741);
    result |= test(__LINE__, I("0x0000000000edcb5433", "0x000000000000005433"), "%#020x", 3989525555U);
    result |= test(__LINE__, I("0X00000000001234ABCD", "0X00000000000000ABCD"), "%#020X", 305441741);
    result |= test(__LINE__, I("0X0000000000EDCB5433", "0X000000000000005433"), "%#020X", 3989525555U);
    result |= test(__LINE__, "Hallo               ", "%0-20s", "Hallo");
    result |= test(__LINE__, "1024                ", "%0-20d", 1024);
    result |= test(__LINE__, "-1024               ", "%0-20d", -1024);
    result |= test(__LINE__, "1024                ", "%0-20i", 1024);
    result |= test(__LINE__, "-1024               ", "%0-20i", -1024);
    result |= test(__LINE__, "1024                ", "%0-20u", 1024);
    result |= test(__LINE__, I("4294966272          ", "64512               "), "%0-20u", 4294966272U);
    result |= test(__LINE__, "777                 ", "%-020o", 511);
    result |= test(__LINE__, I("37777777001         ", "177001              "), "%-020o", 4294966785U);
    result |= test(__LINE__, I("1234abcd            ", "abcd                "), "%-020x", 305441741);
    result |= test(__LINE__, I("edcb5433            ", "5433                "), "%-020x", 3989525555U);
    result |= test(__LINE__, I("1234ABCD            ", "ABCD                "), "%-020X", 305441741);
    result |= test(__LINE__, I("EDCB5433            ", "5433                "), "%-020X", 3989525555U);
    result |= test(__LINE__, "x                   ", "%-020c", 'x');
    result |= test(__LINE__, "               Hallo", "%*s", 20, "Hallo");
    result |= test(__LINE__, "                1024", "%*d", 20, 1024);
    result |= test(__LINE__, "               -1024", "%*d", 20, -1024);
    result |= test(__LINE__, "                1024", "%*i", 20, 1024);
    result |= test(__LINE__, "               -1024", "%*i", 20, -1024);
    result |= test(__LINE__, "                1024", "%*u", 20, 1024);
    result |= test(__LINE__, I("          4294966272", "               64512"), "%*u", 20, 4294966272U);
    result |= test(__LINE__, "                 777", "%*o", 20, 511);
    result |= test(__LINE__, I("         37777777001", "              177001"), "%*o", 20, 4294966785U);
    result |= test(__LINE__, I("            1234abcd", "                abcd"), "%*x", 20, 305441741);
    result |= test(__LINE__, I("            edcb5433", "                5433"), "%*x", 20, 3989525555U);
    result |= test(__LINE__, I("            1234ABCD", "                ABCD"), "%*X", 20, 305441741);
    result |= test(__LINE__, I("            EDCB5433", "                5433"), "%*X", 20, 3989525555U);
    result |= test(__LINE__, "                   x", "%*c", 20, 'x');
    result |= test(__LINE__, "Hallo heimur", "%.20s", "Hallo heimur");
    result |= test(__LINE__, "00000000000000001024", "%.20d", 1024);
    result |= test(__LINE__, "-00000000000000001024", "%.20d", -1024);
    result |= test(__LINE__, "00000000000000001024", "%.20i", 1024);
    result |= test(__LINE__, "-00000000000000001024", "%.20i", -1024);
    result |= test(__LINE__, "00000000000000001024", "%.20u", 1024);
    result |= test(__LINE__, I("00000000004294966272", "00000000000000064512"), "%.20u", 4294966272U);
    result |= test(__LINE__, "00000000000000000777", "%.20o", 511);
    result |= test(__LINE__, I("00000000037777777001", "00000000000000177001"), "%.20o", 4294966785U);
    result |= test(__LINE__, I("0000000000001234abcd", "0000000000000000abcd"), "%.20x", 305441741);
    result |= test(__LINE__, I("000000000000edcb5433", "00000000000000005433"), "%.20x", 3989525555U);
    result |= test(__LINE__, I("0000000000001234ABCD", "0000000000000000ABCD"), "%.20X", 305441741);
    result |= test(__LINE__, I("000000000000EDCB5433", "00000000000000005433"), "%.20X", 3989525555U);
    result |= test(__LINE__, "               Hallo", "%20.5s", "Hallo heimur");
    result |= test(__LINE__, "               01024", "%20.5d", 1024);
    result |= test(__LINE__, "              -01024", "%20.5d", -1024);
    result |= test(__LINE__, "               01024", "%20.5i", 1024);
    result |= test(__LINE__, "              -01024", "%20.5i", -1024);
    result |= test(__LINE__, "               01024", "%20.5u", 1024);
    result |= test(__LINE__, I("          4294966272", "               64512"), "%20.5u", 4294966272U);
    result |= test(__LINE__, "               00777", "%20.5o", 511);
    result |= test(__LINE__, I("         37777777001", "              177001"), "%20.5o", 4294966785U);
    result |= test(__LINE__, I("            1234abcd", "               0abcd"), "%20.5x", 305441741);
    result |= test(__LINE__, I("          00edcb5433", "          0000005433"), "%20.10x", 3989525555U);
    result |= test(__LINE__, I("            1234ABCD", "               0ABCD"), "%20.5X", 305441741);
    result |= test(__LINE__, I("          00EDCB5433", "          0000005433"), "%20.10X", 3989525555U);
    result |= test(__LINE__, "               01024", "%020.5d", 1024);
    result |= test(__LINE__, "              -01024", "%020.5d", -1024);
    result |= test(__LINE__, "               01024", "%020.5i", 1024);
    result |= test(__LINE__, "              -01024", "%020.5i", -1024);
    result |= test(__LINE__, "               01024", "%020.5u", 1024);
    result |= test(__LINE__, I("          4294966272", "               64512"), "%020.5u", 4294966272U);
    result |= test(__LINE__, "               00777", "%020.5o", 511);
    result |= test(__LINE__, I("         37777777001", "              177001"), "%020.5o", 4294966785U);
    result |= test(__LINE__, I("            1234abcd", "               0abcd"), "%020.5x", 305441741);
    result |= test(__LINE__, I("          00edcb5433", "          0000005433"), "%020.10x", 3989525555U);
    result |= test(__LINE__, I("            1234ABCD", "               0ABCD"), "%020.5X", 305441741);
    result |= test(__LINE__, I("          00EDCB5433", "          0000005433"), "%020.10X", 3989525555U);
    result |= test(__LINE__, "", "%.0s", "Hallo heimur");
    result |= test(__LINE__, "                    ", "%20.0s", "Hallo heimur");
    result |= test(__LINE__, "", "%.s", "Hallo heimur");
    result |= test(__LINE__, "                    ", "%20.s", "Hallo heimur");
    result |= test(__LINE__, "                1024", "%20.0d", 1024);
    result |= test(__LINE__, "               -1024", "%20.d", -1024);
    result |= test(__LINE__, "                    ", "%20.d", 0);
    result |= test(__LINE__, "                1024", "%20.0i", 1024);
    result |= test(__LINE__, "               -1024", "%20.i", -1024);
    result |= test(__LINE__, "                    ", "%20.i", 0);
    result |= test(__LINE__, "                1024", "%20.u", 1024);
    result |= test(__LINE__, I("          4294966272", "               64512") , "%20.0u", 4294966272U);
    result |= test(__LINE__, "                    ", "%20.u", 0U);
    result |= test(__LINE__, "                 777", "%20.o", 511);
    result |= test(__LINE__, I("         37777777001", "              177001"), "%20.0o", 4294966785U);
    result |= test(__LINE__, "                    ", "%20.o", 0U);
    result |= test(__LINE__, I("            1234abcd", "                abcd"), "%20.x", 305441741);
    result |= test(__LINE__, I("            edcb5433", "                5433"), "%20.0x", 3989525555U);
    result |= test(__LINE__, "                    ", "%20.x", 0U);
    result |= test(__LINE__, I("            1234ABCD", "                ABCD"), "%20.X", 305441741);
    result |= test(__LINE__, I("            EDCB5433", "                5433"), "%20.0X", 3989525555U);
    result |= test(__LINE__, "                    ", "%20.X", 0U);
    result |= test(__LINE__, "Hallo               ", "% -0+*.*s", 20, 5, "Hallo heimur");
    result |= test(__LINE__, "+01024              ", "% -0+*.*d", 20, 5, 1024);
    result |= test(__LINE__, "-01024              ", "% -0+*.*d", 20, 5, -1024);
    result |= test(__LINE__, "+01024              ", "% -0+*.*i", 20, 5, 1024);
    result |= test(__LINE__, "-01024              ", "% 0-+*.*i", 20, 5, -1024);
    result |= test(__LINE__, "01024               ", "% 0-+*.*u", 20, 5, 1024);
    result |= test(__LINE__, I("4294966272          ", "64512               "), "% 0-+*.*u", 20, 5, 4294966272U);
    result |= test(__LINE__, "00777               ", "%+ -0*.*o", 20, 5, 511);
    result |= test(__LINE__, I("37777777001         ", "177001              "), "%+ -0*.*o", 20, 5, 4294966785U);
    result |= test(__LINE__, I("1234abcd            ", "0abcd               "), "%+ -0*.*x", 20, 5, 305441741);
    result |= test(__LINE__, I("00edcb5433          ", "0000005433          "), "%+ -0*.*x", 20, 10, 3989525555U);
    result |= test(__LINE__, I("1234ABCD            ", "0ABCD               "), "% -+0*.*X", 20, 5, 305441741);
    result |= test(__LINE__, I("00EDCB5433          ", "0000005433          "), "% -+0*.*X", 20, 10, 3989525555U);
    result |= test(__LINE__, "hi x", "%*sx", -3, "hi");
#endif
#ifndef NO_FLOAT
    result |= test(__LINE__, "1.000e-38", "%.3e", 1e-38);
#ifndef LOW_FLOAT
    result |= test(__LINE__, "1.000e-308", "%.3e", 1e-308);
#endif
#endif
#ifndef _NANO_FORMATTED_IO
#ifndef NO_LONGLONG
    result |= test(__LINE__, "1, 1", "%-*.llu, %-*.llu",1,1ULL,1,1ULL);
#endif
#endif
#ifndef NO_FLOAT
    result |= test(__LINE__, "1e-09", "%g", 0.000000001);
    result |= test(__LINE__, "1e-08", "%g", 0.00000001);
    result |= test(__LINE__, "1e-07", "%g", 0.0000001);
    result |= test(__LINE__, "1e-06", "%g", 0.000001);
    result |= test(__LINE__, "0.0001", "%g", 0.0001);
    result |= test(__LINE__, "0.001", "%g", 0.001);
    result |= test(__LINE__, "0.01", "%g", 0.01);
    result |= test(__LINE__, "0.1", "%g", 0.1);
    result |= test(__LINE__, "1", "%g", 1.0);
    result |= test(__LINE__, "10", "%g", 10.0);
    result |= test(__LINE__, "100", "%g", 100.0);
    result |= test(__LINE__, "1000", "%g", 1000.0);
    result |= test(__LINE__, "10000", "%g", 10000.0);
    result |= test(__LINE__, "100000", "%g", 100000.0);
    result |= test(__LINE__, "1e+06", "%g", 1000000.0);
    result |= test(__LINE__, "1e+07", "%g", 10000000.0);
    result |= test(__LINE__, "1e+08", "%g", 100000000.0);
    result |= test(__LINE__, "10.0000", "%#.6g", 10.0);
    result |= test(__LINE__, "10", "%.6g", 10.0);
    result |= test(__LINE__, "10.00000000000000000000", "%#.22g", 10.0);
#endif

    // Regression test for wrong behavior with negative precision in tinystdio
    // this might fail for configurations not using tinystdio, so for a first
    // PR, only run these test for tinystdio.
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__,         "", "%.*s",  0, "123456");
    result |= test(__LINE__,     "1234", "%.*s",  4, "123456");
    result |= test(__LINE__,   "123456", "%.*s", -4, "123456");
    result |= test(__LINE__,       "42", "%.*d",  0, 42);
    result |= test(__LINE__,   "000042", "%.*d",  6, 42);
    result |= test(__LINE__,       "42", "%.*d", -6, 42);
#endif
#ifndef NO_FLOAT
    result |= test(__LINE__,        "0", "%.*f",  0, 0.123);
    result |= test(__LINE__,      "0.1", "%.*f",  1, 0.123);
    result |= test(__LINE__, "0.123000", "%.*f", -1, 0.123);
#endif
#ifdef _WANT_IO_C99_FORMATS
{
    char c[64];
#ifndef _WANT_IO_LONG_LONG
    if (sizeof (intmax_t) <= sizeof(long))
#endif
#ifndef _NANO_FORMATTED_IO
#ifndef NO_WIDTH_PREC
    result |= test(__LINE__, "  42", "%4jd", (intmax_t)42L);
#endif
    result |= test(__LINE__, "64", "%zu", sizeof c);
    result |= test(__LINE__, "12", "%td", (c+12) - c);
#else
    (void) c;
#endif
#ifndef NO_FLOAT
    result |= test(__LINE__, "0x1p+0", "%a", 0x1p+0);
    result |= test(__LINE__, "0x0p+0", "%a", 0.0);
    result |= test(__LINE__, "-0x0p+0", "%a", -0.0);
    result |= test(__LINE__, "0x1.9p+4", "%.1a", 0x1.89p+4);
    result |= test(__LINE__, "0x1.8p+4", "%.1a", 0x1.88p+4);
    result |= test(__LINE__, "0x1.8p+4", "%.1a", 0x1.78p+4);
    result |= test(__LINE__, "0x1.7p+4", "%.1a", 0x1.77p+4);
    result |= test(__LINE__, "0x1.fffffep+126", "%a", (double) 0x1.fffffep+126f);
    result |= test(__LINE__, "0x1.234564p-126", "%a", (double) 0x1.234564p-126f);
    result |= test(__LINE__, "0x1.234566p-126", "%a", (double) 0x1.234566p-126f);
    result |= test(__LINE__, "0X1.FFFFFEP+126", "%A", (double) 0x1.fffffep+126f);
    result |= test(__LINE__, "0X1.234564P-126", "%A", (double) 0x1.234564p-126f);
    result |= test(__LINE__, "0X1.234566P-126", "%A", (double) 0x1.234566p-126f);
    result |= test(__LINE__, "0x1.6p+1", "%.1a", (double) 0x1.6789ap+1f);
    result |= test(__LINE__, "0x1.68p+1", "%.2a", (double) 0x1.6789ap+1f);
    result |= test(__LINE__, "0x1.679p+1", "%.3a", (double) 0x1.6789ap+1f);
    result |= test(__LINE__, "0x1.678ap+1", "%.4a", (double) 0x1.6789ap+1f);
    result |= test(__LINE__, "0x1.6789ap+1", "%.5a", (double) 0x1.6789ap+1f);
    result |= test(__LINE__, "0x1.6789a0p+1", "%.6a", (double) 0x1.6789ap+1f);
    result |= test(__LINE__, "0x1.ffp+1", "%.2a", (double) 0x1.ffp+1f);
    result |= test(__LINE__, "0x2.0p+1", "%.1a", (double) 0x1.ffp+1f);
    result |= test(__LINE__, "nan", "%a", (double) NAN);
    result |= test(__LINE__, "inf", "%a", (double) INFINITY);
    result |= test(__LINE__, "-inf", "%a", (double) -INFINITY);
    result |= test(__LINE__, "NAN", "%A", (double) NAN);
    result |= test(__LINE__, "INF", "%A", (double) INFINITY);
    result |= test(__LINE__, "-INF", "%A", (double) -INFINITY);
#ifdef LOW_FLOAT
#ifdef NORMALIZED_A
    result |= test(__LINE__, "0x1p-149", "%a", 0x1p-149);
#else
    result |= test(__LINE__, "0x0.000002p-126", "%a", 0x1p-149);
#endif
#else
#ifdef NORMALIZED_A
    /* newlib legacy stdio normalizes %a format */
    result |= test(__LINE__, "0x1p-1074", "%a", 0x1p-1074);
#else
    /* glibc and picolibc show denorms like this */
    result |= test(__LINE__, "0x0.0000000000001p-1022", "%a", 0x1p-1074);
#endif
    result |= test(__LINE__, "0x1.fffffffffffffp+1022", "%a", 0x1.fffffffffffffp+1022);
    result |= test(__LINE__, "0x1.23456789abcdep-1022", "%a", 0x1.23456789abcdep-1022);
    result |= test(__LINE__, "0x1.23456789abcdfp-1022", "%a", 0x1.23456789abcdfp-1022);
    result |= test(__LINE__, "0X1.FFFFFFFFFFFFFP+1022", "%A", 0x1.fffffffffffffp+1022);
    result |= test(__LINE__, "0X1.23456789ABCDEP-1022", "%A", 0x1.23456789abcdep-1022);
    result |= test(__LINE__, "0X1.23456789ABCDFP-1022", "%A", 0x1.23456789abcdfp-1022);
#endif
#endif
    /* test %ls for wchar_t string */
    result |= testw(__LINE__, L"foo", L"%.3ls", L"foobar");
    /* test %s for mbchar string */
    result |= testw(__LINE__, L"foo", L"%.3s", "foobar");

    wchar_t wc = 0x1234;

    /* test %lc for wchar_t */
    wchar_t wb[2] = { 0x1234, 0 };
    result |= testw(__LINE__, wb, L"%lc", wc);

    /* make sure %c truncates to char */
    wb[0] = 0x34;
    result |= testw(__LINE__, wb, L"%c", wc);
}
#endif
