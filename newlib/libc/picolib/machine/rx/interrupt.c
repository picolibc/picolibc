/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Renesas Electronics Corporation
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

extern void _start(void);

/* Exception functions */

void        rx_halt_exception(void);

void
rx_halt_exception(void)
{
    for (;;)
        ;
}

#define exception(name)                                                                   \
    void rx_##name##_exception(void) __attribute__((weak, alias("rx_ignore_exception")));

#define exception_halt(name)                                                            \
    void rx_##name##_exception(void) __attribute__((weak, alias("rx_halt_exception")));

exception_halt(privilege);
exception_halt(access);
exception_halt(illegal);
exception_halt(address);
exception_halt(fpu);
exception_halt(nmi);

void _start(void);

#define e(addr, name) [(addr) / 4] = (void (*)(void))rx_##name##_exception

void * const __exception_vector[] __attribute__((section(".rodata.fvectors"))) = {
    e(0x50, privilege), e(0x54, access), e(0x5c, illegal),    e(0x60, address),
    e(0x64, fpu),       e(0x78, nmi),    [0x7c / 4] = _start,
};

void rx_halt_isr(void);

void
rx_halt_isr(void)
{
    for (;;)
        ;
}

#define isr(name) void rx_##name##_isr(void) __attribute__((weak, alias("rx_halt_isr")))

isr(0);
isr(1);
isr(2);
isr(3);
isr(4);
isr(5);
isr(6);
isr(7);
isr(8);
isr(9);
isr(10);
isr(11);
isr(12);
isr(13);
isr(14);
isr(15);
isr(16);
isr(17);
isr(18);
isr(19);
isr(20);
isr(21);
isr(22);
isr(23);
isr(24);
isr(25);
isr(26);
isr(27);
isr(28);
isr(29);
isr(30);
isr(31);
isr(32);
isr(33);
isr(34);
isr(35);
isr(36);
isr(37);
isr(38);
isr(39);
isr(40);
isr(41);
isr(42);
isr(43);
isr(44);
isr(45);
isr(46);
isr(47);
isr(48);
isr(49);
isr(50);
isr(51);
isr(52);
isr(53);
isr(54);
isr(55);
isr(56);
isr(57);
isr(58);
isr(59);
isr(60);
isr(61);
isr(62);
isr(63);
isr(64);
isr(65);
isr(66);
isr(67);
isr(68);
isr(69);
isr(70);
isr(71);
isr(72);
isr(73);
isr(74);
isr(75);
isr(76);
isr(77);
isr(78);
isr(79);
isr(80);
isr(81);
isr(82);
isr(83);
isr(84);
isr(85);
isr(86);
isr(87);
isr(88);
isr(89);
isr(90);
isr(91);
isr(92);
isr(93);
isr(94);
isr(95);
isr(96);
isr(97);
isr(98);
isr(99);
isr(100);
isr(101);
isr(102);
isr(103);
isr(104);
isr(105);
isr(106);
isr(107);
isr(108);
isr(109);
isr(110);
isr(111);
isr(112);
isr(113);
isr(114);
isr(115);
isr(116);
isr(117);
isr(118);
isr(119);
isr(120);
isr(121);
isr(122);
isr(123);
isr(124);
isr(125);
isr(126);
isr(127);
isr(128);
isr(129);
isr(130);
isr(131);
isr(132);
isr(133);
isr(134);
isr(135);
isr(136);
isr(137);
isr(138);
isr(139);
isr(140);
isr(141);
isr(142);
isr(143);
isr(144);
isr(145);
isr(146);
isr(147);
isr(148);
isr(149);
isr(150);
isr(151);
isr(152);
isr(153);
isr(154);
isr(155);
isr(156);
isr(157);
isr(158);
isr(159);
isr(160);
isr(161);
isr(162);
isr(163);
isr(164);
isr(165);
isr(166);
isr(167);
isr(168);
isr(169);
isr(170);
isr(171);
isr(172);
isr(173);
isr(174);
isr(175);
isr(176);
isr(177);
isr(178);
isr(179);
isr(180);
isr(181);
isr(182);
isr(183);
isr(184);
isr(185);
isr(186);
isr(187);
isr(188);
isr(189);
isr(190);
isr(191);
isr(192);
isr(193);
isr(194);
isr(195);
isr(196);
isr(197);
isr(198);
isr(199);
isr(200);
isr(201);
isr(202);
isr(203);
isr(204);
isr(205);
isr(206);
isr(207);
isr(208);
isr(209);
isr(210);
isr(211);
isr(212);
isr(213);
isr(214);
isr(215);
isr(216);
isr(217);
isr(218);
isr(219);
isr(220);
isr(221);
isr(222);
isr(223);
isr(224);
isr(225);
isr(226);
isr(227);
isr(228);
isr(229);
isr(230);
isr(231);
isr(232);
isr(233);
isr(234);
isr(235);
isr(236);
isr(237);
isr(238);
isr(239);
isr(240);
isr(241);
isr(242);
isr(243);
isr(244);
isr(245);
isr(246);
isr(247);
isr(248);
isr(249);
isr(250);
isr(251);
isr(252);
isr(253);
isr(254);
isr(255);

#define i(v) [v] = (void (*)(void))rx_##v##_isr

void * const __interrupt_vector[256] = {
    i(0),   i(1),   i(2),   i(3),   i(4),   i(5),   i(6),   i(7),   i(8),   i(9),   i(10),  i(11),
    i(12),  i(13),  i(14),  i(15),  i(16),  i(17),  i(18),  i(19),  i(20),  i(21),  i(22),  i(23),
    i(24),  i(25),  i(26),  i(27),  i(28),  i(29),  i(30),  i(31),  i(32),  i(33),  i(34),  i(35),
    i(36),  i(37),  i(38),  i(39),  i(40),  i(41),  i(42),  i(43),  i(44),  i(45),  i(46),  i(47),
    i(48),  i(49),  i(50),  i(51),  i(52),  i(53),  i(54),  i(55),  i(56),  i(57),  i(58),  i(59),
    i(60),  i(61),  i(62),  i(63),  i(64),  i(65),  i(66),  i(67),  i(68),  i(69),  i(70),  i(71),
    i(72),  i(73),  i(74),  i(75),  i(76),  i(77),  i(78),  i(79),  i(80),  i(81),  i(82),  i(83),
    i(84),  i(85),  i(86),  i(87),  i(88),  i(89),  i(90),  i(91),  i(92),  i(93),  i(94),  i(95),
    i(96),  i(97),  i(98),  i(99),  i(100), i(101), i(102), i(103), i(104), i(105), i(106), i(107),
    i(108), i(109), i(110), i(111), i(112), i(113), i(114), i(115), i(116), i(117), i(118), i(119),
    i(120), i(121), i(122), i(123), i(124), i(125), i(126), i(127), i(128), i(129), i(130), i(131),
    i(132), i(133), i(134), i(135), i(136), i(137), i(138), i(139), i(140), i(141), i(142), i(143),
    i(144), i(145), i(146), i(147), i(148), i(149), i(150), i(151), i(152), i(153), i(154), i(155),
    i(156), i(157), i(158), i(159), i(160), i(161), i(162), i(163), i(164), i(165), i(166), i(167),
    i(168), i(169), i(170), i(171), i(172), i(173), i(174), i(175), i(176), i(177), i(178), i(179),
    i(180), i(181), i(182), i(183), i(184), i(185), i(186), i(187), i(188), i(189), i(190), i(191),
    i(192), i(193), i(194), i(195), i(196), i(197), i(198), i(199), i(200), i(201), i(202), i(203),
    i(204), i(205), i(206), i(207), i(208), i(209), i(210), i(211), i(212), i(213), i(214), i(215),
    i(216), i(217), i(218), i(219), i(220), i(221), i(222), i(223), i(224), i(225), i(226), i(227),
    i(228), i(229), i(230), i(231), i(232), i(233), i(234), i(235), i(236), i(237), i(238), i(239),
    i(240), i(241), i(242), i(243), i(244), i(245), i(246), i(247), i(248), i(249), i(250), i(251),
    i(252), i(253), i(254), i(255),
};
