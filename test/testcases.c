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

#ifdef TINY_STDIO
# ifdef PICOLIBC_FLOAT_PRINTF_SCANF
#  define LOW_FLOAT
# endif
# ifdef PICOLIBC_INTEGER_PRINTF_SCANF
#  define NO_FLOAT
#  ifndef _WANT_IO_LONG_LONG
#   define NO_LONGLONG
#  endif
# endif
#else
# ifdef NO_FLOATING_POINT
#  define NO_FLOAT
# endif
# ifndef _WANT_IO_LONG_LONG
#  define NO_LONGLONG
# endif
#endif

/* XXX This code generated automatically by gen-testcases.hs
   from ../../printf-tests.txt . You probably do not want to
   manually edit this file. */
#ifndef NO_FLOAT
    result |= test(0, "0", "%.7g", 0.0);
    result |= test(1, "0.33", "%.*f", 2, 0.33333333);
#endif
    result |= test(2, "foo", "%.3s", "foobar");
    result |= test(3, "     00004", "%10.5d", 4);
    result |= test(4, " 42", "% d", 42);
    result |= test(5, "-42", "% d", -42);
    result |= test(6, "   42", "% 5d", 42);
    result |= test(7, "  -42", "% 5d", -42);
    result |= test(8, "             42", "% 15d", 42);
    result |= test(9, "            -42", "% 15d", -42);
    result |= test(10, "+42", "%+d", 42);
    result |= test(11, "-42", "%+d", -42);
    result |= test(12, "  +42", "%+5d", 42);
    result |= test(13, "  -42", "%+5d", -42);
    result |= test(14, "            +42", "%+15d", 42);
    result |= test(15, "            -42", "%+15d", -42);
    result |= test(16, "42", "%0d", 42);
    result |= test(17, "-42", "%0d", -42);
    result |= test(18, "00042", "%05d", 42);
    result |= test(19, "-0042", "%05d", -42);
    result |= test(20, "000000000000042", "%015d", 42);
    result |= test(21, "-00000000000042", "%015d", -42);
    result |= test(22, "42", "%-d", 42);
    result |= test(23, "-42", "%-d", -42);
    result |= test(24, "42   ", "%-5d", 42);
    result |= test(25, "-42  ", "%-5d", -42);
    result |= test(26, "42             ", "%-15d", 42);
    result |= test(27, "-42            ", "%-15d", -42);
    result |= test(28, "42", "%-0d", 42);
    result |= test(29, "-42", "%-0d", -42);
    result |= test(30, "42   ", "%-05d", 42);
    result |= test(31, "-42  ", "%-05d", -42);
    result |= test(32, "42             ", "%-015d", 42);
    result |= test(33, "-42            ", "%-015d", -42);
    result |= test(34, "42", "%0-d", 42);
    result |= test(35, "-42", "%0-d", -42);
    result |= test(36, "42   ", "%0-5d", 42);
    result |= test(37, "-42  ", "%0-5d", -42);
    result |= test(38, "42             ", "%0-15d", 42);
    result |= test(39, "-42            ", "%0-15d", -42);
#ifndef NO_FLOAT
    result |= test(43, "42.90", "%.2f", 42.8952);
    result |= test(44, "42.90", "%.2F", 42.8952);
#ifdef LOW_FLOAT
    result |= test(45, "42.89520", "%.5f", 42.8952);
#else
    result |= test(45, "42.8952000000", "%.10f", 42.8952);
#endif
    result |= test(46, "42.90", "%1.2f", 42.8952);
    result |= test(47, " 42.90", "%6.2f", 42.8952);
    result |= test(49, "+42.90", "%+6.2f", 42.8952);
#ifdef LOW_FLOAT
    result |= test(50, "42.89520", "%5.5f", 42.8952);
#else
    result |= test(50, "42.8952000000", "%5.10f", 42.8952);
#endif
#endif /* NO_FLOAT */
    /* 51: anti-test */
    /* 52: anti-test */
    /* 53: excluded for C */
//    result |= test(55, "Hot Pocket", "%1$s %2$s", "Hot", "Pocket");
//    result |= test(56, "12.0 Hot Pockets", "%1$.1f %2$s %3$ss", 12.0, "Hot", "Pocket");
    /* 58: anti-test */
#ifdef TINY_STDIO
    result |= test(59, "%(foo", "%(foo");
#endif
    result |= test(60, " foo", "%*s", 4, "foo");
#ifndef NO_FLOAT
    result |= test(61, "      3.14", "%*.*f", 10, 2, 3.14159265);
    result |= test(63, "3.14      ", "%-*.*f", 10, 2, 3.14159265);
#endif
    /* 64: anti-test */
    /* 65: anti-test */
    result |= test(66, "+hello+", "+%s+", "hello");
    result |= test(67, "+10+", "+%d+", 10);
    result |= test(68, "a", "%c", 'a');
    result |= test(69, " ", "%c", 32);
    result |= test(70, "$", "%c", 36);
    result |= test(71, "10", "%d", 10);
    /* 72: anti-test */
    /* 73: anti-test */
    /* 74: excluded for C */
    /* 75: excluded for C */
#ifndef NO_FLOAT
#ifdef LOW_FLOAT
    result |= test(76, "         +7.894561e+08", "%+#22.6e", 7.89456123e8);
    result |= test(77, "7.894561e+08          ", "%-#22.6e", 7.89456123e8);
    result |= test(78, "          7.894561e+08", "%#22.6e", 7.89456123e8);
#else
    result |= test(76, "+7.894561230000000e+08", "%+#22.15e", 7.89456123e8);
    result |= test(77, "7.894561230000000e+08 ", "%-#22.15e", 7.89456123e8);
    result |= test(78, " 7.894561230000000e+08", "%#22.15e", 7.89456123e8);
#endif
    result |= test(79, "8.e+08", "%#1.1g", 7.89456123e8);
#endif
#ifndef NO_LONGLONG
    result |= test(81, "    +100", "%+8lld", 100LL);
    result |= test(82, "+00000100", "%+.8lld", 100LL);
    result |= test(83, " +00000100", "%+10.8lld", 100LL);
#ifdef TINY_STDIO
    result |= test(84, "%_1lld", "%_1lld", 100LL);
#endif
    result |= test(85, "-00100", "%-1.5lld", -100LL);
    result |= test(86, "  100", "%5lld", 100LL);
    result |= test(87, " -100", "%5lld", -100LL);
    result |= test(88, "100  ", "%-5lld", 100LL);
    result |= test(89, "-100 ", "%-5lld", -100LL);
    result |= test(90, "00100", "%-.5lld", 100LL);
    result |= test(91, "-00100", "%-.5lld", -100LL);
    result |= test(92, "00100   ", "%-8.5lld", 100LL);
    result |= test(93, "-00100  ", "%-8.5lld", -100LL);
    result |= test(94, "00100", "%05lld", 100LL);
    result |= test(95, "-0100", "%05lld", -100LL);
    result |= test(96, " 100", "% lld", 100LL);
    result |= test(97, "-100", "% lld", -100LL);
    result |= test(98, "  100", "% 5lld", 100LL);
    result |= test(99, " -100", "% 5lld", -100LL);
    result |= test(100, " 00100", "% .5lld", 100LL);
    result |= test(101, "-00100", "% .5lld", -100LL);
    result |= test(102, "   00100", "% 8.5lld", 100LL);
    result |= test(103, "  -00100", "% 8.5lld", -100LL);
    result |= test(104, "", "%.0lld", 0LL);
    result |= test(105, " 0x00ffffffffffffff9c", "%#+21.18llx", -100LL);
    result |= test(106, "0001777777777777777777634", "%#.25llo", -100LL);
    result |= test(107, " 01777777777777777777634", "%#+24.20llo", -100LL);
    result |= test(108, "0X00000FFFFFFFFFFFFFF9C", "%#+18.21llX", -100LL);
    result |= test(109, "001777777777777777777634", "%#+20.24llo", -100LL);
    result |= test(110, "   0018446744073709551615", "%#+25.22llu", -1LL);
    result |= test(111, "   0018446744073709551615", "%#+25.22llu", -1LL);
    result |= test(112, "     0000018446744073709551615", "%#+30.25llu", -1LL);
    result |= test(113, "  -0000000000000000000001", "%+#25.22lld", -1LL);
    result |= test(114, "00144   ", "%#-8.5llo", 100LL);
    result |= test(115, "+00100  ", "%#-+ 08.5lld", 100LL);
    result |= test(116, "+00100  ", "%#-+ 08.5lld", 100LL);
    result |= test(117, "0000000000000000000000000000000000000001", "%.40lld", 1LL);
    result |= test(118, " 0000000000000000000000000000000000000001", "% .40lld", 1LL);
#endif
    result |= test(119, " 0000000000000000000000000000000000000001", "% .40d", 1);
    /* 121: excluded for C */
    /* 124: excluded for C */
    result |= test(125, " 1", "% d", 1);
    result |= test(126, "+1", "%+ d", 1);
    result |= test(129, "0x0000000001", "%#012x", 1);
    result |= test(130, "0x00000001", "%#04.8x", 1);
    result |= test(131, "0x01    ", "%#-08.2x", 1);
    result |= test(132, "00000001", "%#08o", 1);
    result |= test(133, "0x39", "%p", (void *)57ULL);
    result |= test(137, "0x39", "%p", (void *)57U);
    result |= test(142, "f", "%.1s", "foo");
    result |= test(143, "f", "%.*s", 1, "foo");
    result |= test(144, "foo  ", "%*s", -5, "foo");
    result |= test(145, "hello", "hello");
#ifdef TINY_STDIO
    result |= test(147, "%b", "%b");
#endif
    result |= test(148, "  a", "%3c", 'a');
    result |= test(149, "1234", "%3d", 1234);
    /* 150: excluded for C */
    result |= test(152, "2", "%-1d", 2);
#ifndef NO_FLOAT
    result |= test(153, "8.6000", "%2.4f", 8.6);
    result |= test(154, "0.600000", "%0f", 0.6);
    result |= test(155, "1", "%.0f", 0.6);
    result |= test(156, "8.6000e+00", "%2.4e", 8.6);
    result |= test(157, " 8.6000e+00", "% 2.4e", 8.6);
    result |= test(159, "-8.6000e+00", "% 2.4e", -8.6);
    result |= test(160, "+8.6000e+00", "%+2.4e", 8.6);
    result |= test(161, "8.6", "%2.4g", 8.6);
#endif
    result |= test(162, "-1", "%-i", -1);
    result |= test(163, "1", "%-i", 1);
    result |= test(164, "+1", "%+i", 1);
    result |= test(165, "12", "%o", 10);
    /* 166: excluded for C */
    /* 167: excluded for C */
#ifdef TINY_STDIO
    result |= test(168, "(null)", "%s", NULL);
#endif
    result |= test(169, "%%%%", "%s", "%%%%");
    result |= test(170, "4294967295", "%u", -1);
#ifdef TINY_STDIO
    result |= test(171, "%w", "%w", -1);
#endif
    /* 172: excluded for C */
    /* 173: excluded for C */
    /* 174: excluded for C */
#ifdef TINY_STDIO
    result |= test(176, "%H", "%H", -1);
#endif
    result |= test(177, "%0", "%%0");
    result |= test(178, "2345", "%hx", 74565);
    result |= test(179, "61", "%hhx", 'a');
    result |= test(180, "61", "%hhx", 'a' + 256);
    result |= test(181, "Hallo heimur", "Hallo heimur");
    result |= test(182, "Hallo heimur", "%s", "Hallo heimur");
    result |= test(183, "1024", "%d", 1024);
    result |= test(184, "-1024", "%d", -1024);
    result |= test(185, "1024", "%i", 1024);
    result |= test(186, "-1024", "%i", -1024);
    result |= test(187, "1024", "%u", 1024);
    result |= test(188, "4294966272", "%u", 4294966272U);
    result |= test(189, "777", "%o", 511);
    result |= test(190, "37777777001", "%o", 4294966785U);
    result |= test(191, "1234abcd", "%x", 305441741);
    result |= test(192, "edcb5433", "%x", 3989525555U);
    result |= test(193, "1234ABCD", "%X", 305441741);
    result |= test(194, "EDCB5433", "%X", 3989525555U);
    result |= test(195, "x", "%c", 'x');
    result |= test(196, "%", "%%");
    result |= test(197, "Hallo heimur", "%+s", "Hallo heimur");
    result |= test(198, "+1024", "%+d", 1024);
    result |= test(199, "-1024", "%+d", -1024);
    result |= test(200, "+1024", "%+i", 1024);
    result |= test(201, "-1024", "%+i", -1024);
    result |= test(202, "1024", "%+u", 1024);
    result |= test(203, "4294966272", "%+u", 4294966272U);
    result |= test(204, "777", "%+o", 511);
    result |= test(205, "37777777001", "%+o", 4294966785U);
    result |= test(206, "1234abcd", "%+x", 305441741);
    result |= test(207, "edcb5433", "%+x", 3989525555U);
    result |= test(208, "1234ABCD", "%+X", 305441741);
    result |= test(209, "EDCB5433", "%+X", 3989525555U);
    result |= test(210, "x", "%+c", 'x');
    result |= test(211, "Hallo heimur", "% s", "Hallo heimur");
    result |= test(212, " 1024", "% d", 1024);
    result |= test(213, "-1024", "% d", -1024);
    result |= test(214, " 1024", "% i", 1024);
    result |= test(215, "-1024", "% i", -1024);
    result |= test(216, "1024", "% u", 1024);
    result |= test(217, "4294966272", "% u", 4294966272U);
    result |= test(218, "777", "% o", 511);
    result |= test(219, "37777777001", "% o", 4294966785U);
    result |= test(220, "1234abcd", "% x", 305441741);
    result |= test(221, "edcb5433", "% x", 3989525555U);
    result |= test(222, "1234ABCD", "% X", 305441741);
    result |= test(223, "EDCB5433", "% X", 3989525555U);
    result |= test(224, "x", "% c", 'x');
    result |= test(225, "Hallo heimur", "%+ s", "Hallo heimur");
    result |= test(226, "+1024", "%+ d", 1024);
    result |= test(227, "-1024", "%+ d", -1024);
    result |= test(228, "+1024", "%+ i", 1024);
    result |= test(229, "-1024", "%+ i", -1024);
    result |= test(230, "1024", "%+ u", 1024);
    result |= test(231, "4294966272", "%+ u", 4294966272U);
    result |= test(232, "777", "%+ o", 511);
    result |= test(233, "37777777001", "%+ o", 4294966785U);
    result |= test(234, "1234abcd", "%+ x", 305441741);
    result |= test(235, "edcb5433", "%+ x", 3989525555U);
    result |= test(236, "1234ABCD", "%+ X", 305441741);
    result |= test(237, "EDCB5433", "%+ X", 3989525555U);
    result |= test(238, "x", "%+ c", 'x');
    result |= test(239, "0777", "%#o", 511);
    result |= test(240, "037777777001", "%#o", 4294966785U);
    result |= test(241, "0x1234abcd", "%#x", 305441741);
    result |= test(242, "0xedcb5433", "%#x", 3989525555U);
    result |= test(243, "0X1234ABCD", "%#X", 305441741);
    result |= test(244, "0XEDCB5433", "%#X", 3989525555U);
    result |= test(245, "0", "%#o", 0U);
    result |= test(246, "0", "%#x", 0U);
    result |= test(247, "0", "%#X", 0U);
    result |= test(248, "Hallo heimur", "%1s", "Hallo heimur");
    result |= test(249, "1024", "%1d", 1024);
    result |= test(250, "-1024", "%1d", -1024);
    result |= test(251, "1024", "%1i", 1024);
    result |= test(252, "-1024", "%1i", -1024);
    result |= test(253, "1024", "%1u", 1024);
    result |= test(254, "4294966272", "%1u", 4294966272U);
    result |= test(255, "777", "%1o", 511);
    result |= test(256, "37777777001", "%1o", 4294966785U);
    result |= test(257, "1234abcd", "%1x", 305441741);
    result |= test(258, "edcb5433", "%1x", 3989525555U);
    result |= test(259, "1234ABCD", "%1X", 305441741);
    result |= test(260, "EDCB5433", "%1X", 3989525555U);
    result |= test(261, "x", "%1c", 'x');
    result |= test(262, "               Hallo", "%20s", "Hallo");
    result |= test(263, "                1024", "%20d", 1024);
    result |= test(264, "               -1024", "%20d", -1024);
    result |= test(265, "                1024", "%20i", 1024);
    result |= test(266, "               -1024", "%20i", -1024);
    result |= test(267, "                1024", "%20u", 1024);
    result |= test(268, "          4294966272", "%20u", 4294966272U);
    result |= test(269, "                 777", "%20o", 511);
    result |= test(270, "         37777777001", "%20o", 4294966785U);
    result |= test(271, "            1234abcd", "%20x", 305441741);
    result |= test(272, "            edcb5433", "%20x", 3989525555U);
    result |= test(273, "            1234ABCD", "%20X", 305441741);
    result |= test(274, "            EDCB5433", "%20X", 3989525555U);
    result |= test(275, "                   x", "%20c", 'x');
    result |= test(276, "Hallo               ", "%-20s", "Hallo");
    result |= test(277, "1024                ", "%-20d", 1024);
    result |= test(278, "-1024               ", "%-20d", -1024);
    result |= test(279, "1024                ", "%-20i", 1024);
    result |= test(280, "-1024               ", "%-20i", -1024);
    result |= test(281, "1024                ", "%-20u", 1024);
    result |= test(282, "4294966272          ", "%-20u", 4294966272U);
    result |= test(283, "777                 ", "%-20o", 511);
    result |= test(284, "37777777001         ", "%-20o", 4294966785U);
    result |= test(285, "1234abcd            ", "%-20x", 305441741);
    result |= test(286, "edcb5433            ", "%-20x", 3989525555U);
    result |= test(287, "1234ABCD            ", "%-20X", 305441741);
    result |= test(288, "EDCB5433            ", "%-20X", 3989525555U);
    result |= test(289, "x                   ", "%-20c", 'x');
    result |= test(290, "00000000000000001024", "%020d", 1024);
    result |= test(291, "-0000000000000001024", "%020d", -1024);
    result |= test(292, "00000000000000001024", "%020i", 1024);
    result |= test(293, "-0000000000000001024", "%020i", -1024);
    result |= test(294, "00000000000000001024", "%020u", 1024);
    result |= test(295, "00000000004294966272", "%020u", 4294966272U);
    result |= test(296, "00000000000000000777", "%020o", 511);
    result |= test(297, "00000000037777777001", "%020o", 4294966785U);
    result |= test(298, "0000000000001234abcd", "%020x", 305441741);
    result |= test(299, "000000000000edcb5433", "%020x", 3989525555U);
    result |= test(300, "0000000000001234ABCD", "%020X", 305441741);
    result |= test(301, "000000000000EDCB5433", "%020X", 3989525555U);
    result |= test(302, "                0777", "%#20o", 511);
    result |= test(303, "        037777777001", "%#20o", 4294966785U);
    result |= test(304, "          0x1234abcd", "%#20x", 305441741);
    result |= test(305, "          0xedcb5433", "%#20x", 3989525555U);
    result |= test(306, "          0X1234ABCD", "%#20X", 305441741);
    result |= test(307, "          0XEDCB5433", "%#20X", 3989525555U);
    result |= test(308, "00000000000000000777", "%#020o", 511);
    result |= test(309, "00000000037777777001", "%#020o", 4294966785U);
    result |= test(310, "0x00000000001234abcd", "%#020x", 305441741);
    result |= test(311, "0x0000000000edcb5433", "%#020x", 3989525555U);
    result |= test(312, "0X00000000001234ABCD", "%#020X", 305441741);
    result |= test(313, "0X0000000000EDCB5433", "%#020X", 3989525555U);
    result |= test(314, "Hallo               ", "%0-20s", "Hallo");
    result |= test(315, "1024                ", "%0-20d", 1024);
    result |= test(316, "-1024               ", "%0-20d", -1024);
    result |= test(317, "1024                ", "%0-20i", 1024);
    result |= test(318, "-1024               ", "%0-20i", -1024);
    result |= test(319, "1024                ", "%0-20u", 1024);
    result |= test(320, "4294966272          ", "%0-20u", 4294966272U);
    result |= test(321, "777                 ", "%-020o", 511);
    result |= test(322, "37777777001         ", "%-020o", 4294966785U);
    result |= test(323, "1234abcd            ", "%-020x", 305441741);
    result |= test(324, "edcb5433            ", "%-020x", 3989525555U);
    result |= test(325, "1234ABCD            ", "%-020X", 305441741);
    result |= test(326, "EDCB5433            ", "%-020X", 3989525555U);
    result |= test(327, "x                   ", "%-020c", 'x');
    result |= test(328, "               Hallo", "%*s", 20, "Hallo");
    result |= test(329, "                1024", "%*d", 20, 1024);
    result |= test(330, "               -1024", "%*d", 20, -1024);
    result |= test(331, "                1024", "%*i", 20, 1024);
    result |= test(332, "               -1024", "%*i", 20, -1024);
    result |= test(333, "                1024", "%*u", 20, 1024);
    result |= test(334, "          4294966272", "%*u", 20, 4294966272U);
    result |= test(335, "                 777", "%*o", 20, 511);
    result |= test(336, "         37777777001", "%*o", 20, 4294966785U);
    result |= test(337, "            1234abcd", "%*x", 20, 305441741);
    result |= test(338, "            edcb5433", "%*x", 20, 3989525555U);
    result |= test(339, "            1234ABCD", "%*X", 20, 305441741);
    result |= test(340, "            EDCB5433", "%*X", 20, 3989525555U);
    result |= test(341, "                   x", "%*c", 20, 'x');
    result |= test(342, "Hallo heimur", "%.20s", "Hallo heimur");
    result |= test(343, "00000000000000001024", "%.20d", 1024);
    result |= test(344, "-00000000000000001024", "%.20d", -1024);
    result |= test(345, "00000000000000001024", "%.20i", 1024);
    result |= test(346, "-00000000000000001024", "%.20i", -1024);
    result |= test(347, "00000000000000001024", "%.20u", 1024);
    result |= test(348, "00000000004294966272", "%.20u", 4294966272U);
    result |= test(349, "00000000000000000777", "%.20o", 511);
    result |= test(350, "00000000037777777001", "%.20o", 4294966785U);
    result |= test(351, "0000000000001234abcd", "%.20x", 305441741);
    result |= test(352, "000000000000edcb5433", "%.20x", 3989525555U);
    result |= test(353, "0000000000001234ABCD", "%.20X", 305441741);
    result |= test(354, "000000000000EDCB5433", "%.20X", 3989525555U);
    result |= test(355, "               Hallo", "%20.5s", "Hallo heimur");
    result |= test(356, "               01024", "%20.5d", 1024);
    result |= test(357, "              -01024", "%20.5d", -1024);
    result |= test(358, "               01024", "%20.5i", 1024);
    result |= test(359, "              -01024", "%20.5i", -1024);
    result |= test(360, "               01024", "%20.5u", 1024);
    result |= test(361, "          4294966272", "%20.5u", 4294966272U);
    result |= test(362, "               00777", "%20.5o", 511);
    result |= test(363, "         37777777001", "%20.5o", 4294966785U);
    result |= test(364, "            1234abcd", "%20.5x", 305441741);
    result |= test(365, "          00edcb5433", "%20.10x", 3989525555U);
    result |= test(366, "            1234ABCD", "%20.5X", 305441741);
    result |= test(367, "          00EDCB5433", "%20.10X", 3989525555U);
    result |= test(369, "               01024", "%020.5d", 1024);
    result |= test(370, "              -01024", "%020.5d", -1024);
    result |= test(371, "               01024", "%020.5i", 1024);
    result |= test(372, "              -01024", "%020.5i", -1024);
    result |= test(373, "               01024", "%020.5u", 1024);
    result |= test(374, "          4294966272", "%020.5u", 4294966272U);
    result |= test(375, "               00777", "%020.5o", 511);
    result |= test(376, "         37777777001", "%020.5o", 4294966785U);
    result |= test(377, "            1234abcd", "%020.5x", 305441741);
    result |= test(378, "          00edcb5433", "%020.10x", 3989525555U);
    result |= test(379, "            1234ABCD", "%020.5X", 305441741);
    result |= test(380, "          00EDCB5433", "%020.10X", 3989525555U);
    result |= test(381, "", "%.0s", "Hallo heimur");
    result |= test(382, "                    ", "%20.0s", "Hallo heimur");
    result |= test(383, "", "%.s", "Hallo heimur");
    result |= test(384, "                    ", "%20.s", "Hallo heimur");
    result |= test(385, "                1024", "%20.0d", 1024);
    result |= test(386, "               -1024", "%20.d", -1024);
    result |= test(387, "                    ", "%20.d", 0);
    result |= test(388, "                1024", "%20.0i", 1024);
    result |= test(389, "               -1024", "%20.i", -1024);
    result |= test(390, "                    ", "%20.i", 0);
    result |= test(391, "                1024", "%20.u", 1024);
    result |= test(392, "          4294966272", "%20.0u", 4294966272U);
    result |= test(393, "                    ", "%20.u", 0U);
    result |= test(394, "                 777", "%20.o", 511);
    result |= test(395, "         37777777001", "%20.0o", 4294966785U);
    result |= test(396, "                    ", "%20.o", 0U);
    result |= test(397, "            1234abcd", "%20.x", 305441741);
    result |= test(398, "            edcb5433", "%20.0x", 3989525555U);
    result |= test(399, "                    ", "%20.x", 0U);
    result |= test(400, "            1234ABCD", "%20.X", 305441741);
    result |= test(401, "            EDCB5433", "%20.0X", 3989525555U);
    result |= test(402, "                    ", "%20.X", 0U);
    result |= test(403, "Hallo               ", "% -0+*.*s", 20, 5, "Hallo heimur");
    result |= test(404, "+01024              ", "% -0+*.*d", 20, 5, 1024);
    result |= test(405, "-01024              ", "% -0+*.*d", 20, 5, -1024);
    result |= test(406, "+01024              ", "% -0+*.*i", 20, 5, 1024);
    result |= test(407, "-01024              ", "% 0-+*.*i", 20, 5, -1024);
    result |= test(408, "01024               ", "% 0-+*.*u", 20, 5, 1024);
    result |= test(409, "4294966272          ", "% 0-+*.*u", 20, 5, 4294966272U);
    result |= test(410, "00777               ", "%+ -0*.*o", 20, 5, 511);
    result |= test(411, "37777777001         ", "%+ -0*.*o", 20, 5, 4294966785U);
    result |= test(412, "1234abcd            ", "%+ -0*.*x", 20, 5, 305441741);
    result |= test(413, "00edcb5433          ", "%+ -0*.*x", 20, 10, 3989525555U);
    result |= test(414, "1234ABCD            ", "% -+0*.*X", 20, 5, 305441741);
    result |= test(415, "00EDCB5433          ", "% -+0*.*X", 20, 10, 3989525555U);
    result |= test(416, "hi x", "%*sx", -3, "hi");
#ifndef NO_FLOAT
    result |= test(417, "1.000e-38", "%.3e", 1e-38);
#ifndef LOW_FLOAT
    result |= test(418, "1.000e-308", "%.3e", 1e-308);
#endif
#endif
    result |= test(419, "1, 1", "%-*.llu, %-*.llu",1,(int64_t)1,1,(int64_t)1);
#ifndef NO_FLOAT
    result |= test(420, "1e-09", "%g", 0.000000001);
    result |= test(421, "1e-08", "%g", 0.00000001);
    result |= test(422, "1e-07", "%g", 0.0000001);
    result |= test(423, "1e-06", "%g", 0.000001);
    result |= test(424, "0.0001", "%g", 0.0001);
    result |= test(425, "0.001", "%g", 0.001);
    result |= test(426, "0.01", "%g", 0.01);
    result |= test(427, "0.1", "%g", 0.1);
    result |= test(428, "1", "%g", 1.0);
    result |= test(429, "10", "%g", 10.0);
    result |= test(430, "100", "%g", 100.0);
    result |= test(431, "1000", "%g", 1000.0);
    result |= test(432, "10000", "%g", 10000.0);
    result |= test(433, "100000", "%g", 100000.0);
    result |= test(434, "1e+06", "%g", 1000000.0);
    result |= test(435, "1e+07", "%g", 10000000.0);
    result |= test(436, "1e+08", "%g", 100000000.0);
    result |= test(437, "10.0000", "%#.6g", 10.0);
    result |= test(438, "10", "%.6g", 10.0);
    result |= test(439, "10.00000000000000000000", "%#.22g", 10.0);
#endif

    // Regression test for wrong behavior with negative precision in tinystdio
    // this might fail for configurations not using tinystdio, so for a first
    // PR, only run these test for tinystdio.
    result |= test(440,         "", "%.*s",  0, "123456");
    result |= test(441,     "1234", "%.*s",  4, "123456");
    result |= test(442,   "123456", "%.*s", -4, "123456");
    result |= test(443,       "42", "%.*d",  0, 42);
    result |= test(444,   "000042", "%.*d",  6, 42);
    result |= test(445,       "42", "%.*d", -6, 42);
#ifndef NO_FLOAT
    result |= test(446,        "0", "%.*f",  0, 0.123);
    result |= test(447,      "0.1", "%.*f",  1, 0.123);
    result |= test(448, "0.123000", "%.*f", -1, 0.123);
#endif
#ifdef _WANT_IO_C99_FORMATS
{
    char c[64];
#ifndef _WANT_IO_LONG_LONG
    if (sizeof (intmax_t) <= sizeof(long))
#endif
    result |= test(449, "  42", "%4jd", (intmax_t)42L);
    result |= test(450, "64", "%zu", sizeof c);
    result |= test(451, "12", "%td", (c+12) - c);
#ifndef NO_FLOAT
#ifdef LOW_FLOAT
    result |= test(452, "0x1.000000p+0", "%a", (double) 0x1.0p+0f);
    result |= test(453, "0x0.000002p-126", "%a", (double) 0x1.000000p-149f);
    result |= test(454, "0x0.000000p+0", "%a", (double) 0.0f);
    result |= test(455, "0x1.fffffep+126", "%a", (double) 0x1.fffffep+126f);
    result |= test(456, "0x1.234564p-126", "%a", (double) 0x1.234564p-126f);
    result |= test(457, "0x1.234566p-126", "%a", (double) 0x1.234566p-126f);
#else
#ifdef TINY_STDIO
    result |= test(452, "0x1.0000000000000p+0", "%a", 0x1.0p+0);
    result |= test(453, "0x0.0000000000001p-1022", "%a", 0x1.0000000000000p-1074);
    result |= test(454, "0x0.0000000000000p+0", "%a", 0.0);
    result |= test(454, "-0x0.0000000000000p+0", "%a", -0.0);
#else
    result |= test(452, "0x1p+0", "%a", 0x1.0p+0);
    result |= test(453, "0x1p-1074", "%a", 0x1.0000000000000p-1074);
    result |= test(454, "0x0p+0", "%a", 0.0);
#endif
    result |= test(455, "0x1.fffffffffffffp+1022", "%a", 0x1.fffffffffffffp+1022);
    result |= test(456, "0x1.23456789abcdep-1022", "%a", 0x1.23456789abcdep-1022);
    result |= test(457, "0x1.23456789abcdfp-1022", "%a", 0x1.23456789abcdfp-1022);
#endif
#endif
}
#endif
