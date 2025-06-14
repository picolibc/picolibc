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

#define WANT_FLOAT128

#define MAX_ARG 1756

/* 50 coefficients */
#define COEFFS {                                                \
        float_128(0x1p0),                                        \
        float_128(-0x1.2788cfc6fb618f49a37c7f0202a6p-1),         \
        float_128(0x1.fa658c23b15787764ad196a08943p-1),          \
        float_128(-0x1.d0a118f324b62f62d85be450d849p-1),         \
        float_128(0x1.f6a51055096b53f57c9ee773b59ep-1),          \
        float_128(-0x1.f6c80ec38b67a8d80e1ab8fa6a95p-1),         \
        float_128(0x1.fc7e0a6eb310af7dee704f09d476p-1),          \
        float_128(-0x1.fdf3f157b7a395bf459f015cb2c6p-1),         \
        float_128(0x1.ff07b5a17ff6b99202c7486cadb5p-1),          \
        float_128(-0x1.ff803d68a0bd3f8d2d7800b5fb8cp-1),         \
        float_128(0x1.ffc0841d585a2abde75ef5b72a5ep-1),          \
        float_128(-0x1.ffe018c484f47c938f494302ca6dp-1),         \
        float_128(0x1.fff00b768f1c665116b97f85f3dep-1),          \
        float_128(-0x1.fff8035584df070798c1a5259458p-1),         \
        float_128(0x1.fffc012f94cca805daa8e19d8542p-1),          \
        float_128(-0x1.fffe0062ab5173cd60867ce6480ep-1),         \
        float_128(0x1.ffff002110f798418ffa4b79e7ddp-1),          \
        float_128(-0x1.ffff8008ddb9158af2c76d629a39p-1),         \
        float_128(0x1.ffffbff079b3a356671841f86f92p-1),          \
        float_128(-0x1.ffffdf70d67a7323db8edbb3dd6bp-1),         \
        float_128(0x1.ffffec5271233146fa68c0c469acp-1),          \
        float_128(-0x1.ffffe2f09d30a441565bb36dc0cp-1),          \
        float_128(0x1.ffff9168263bc998a44387d7f708p-1),          \
        float_128(-0x1.fffe1e03efcf1ae367208e70ff94p-1),         \
        float_128(0x1.fff8728ad592bb8a672f7ec34615p-1),          \
        float_128(-0x1.ffe4b6997139df04450a290ee06cp-1),         \
        float_128(0x1.ffa726757fee92d78d0b76dff26cp-1),          \
        float_128(-0x1.fefa8e713109c04b3607cac3456p-1),          \
        float_128(0x1.fd46f2bce32f1d9ab8e21b9b349cp-1),          \
        float_128(-0x1.f967f3fc4b57b5c1f3d4c245ccddp-1),         \
        float_128(0x1.f176301dcbb42cf03297562644abp-1),          \
        float_128(-0x1.e2bf8b7276f617d654065cc45382p-1),         \
        float_128(0x1.ca28144522ab2a9e7bc0e6d2e11cp-1),          \
        float_128(-0x1.a5147ef254b51e7f76707764a218p-1),         \
        float_128(0x1.72b2905d51365e0bf191627401fdp-1),          \
        float_128(-0x1.351227d17aa105fc4bf2f4a1cb5p-1),          \
        float_128(0x1.e2ae628ca7b691498e0051354f3cp-2),          \
        float_128(-0x1.5d3a02c575bf5221088e977edf6fp-2),         \
        float_128(0x1.cf70ebfdb350e7f710d1ca6e52efp-3),          \
        float_128(-0x1.1731dee63733b46f0aa9f1527043p-3),         \
        float_128(0x1.2e5a2d0acf89c5edef8a7e0c9a2dp-4),          \
        float_128(-0x1.232d05287202ac051700e4f07426p-5),         \
        float_128(0x1.ece4ae60fab4ba6d44fc4ce1a4abp-7),          \
        float_128(-0x1.69ada0dea446b8898021b4ac2f3ap-8),         \
        float_128(0x1.c49c595bee5a64fc5304c6d3119ep-10),         \
        float_128(-0x1.d8dfddf18a189f3d639e2e11ad06p-12),        \
        float_128(0x1.90e2c3c23a94a90866b4287a58a7p-14),         \
        float_128(-0x1.08b50caa073e2e51f4e1d28e6798p-16),        \
        float_128(0x1.fe85dfa590a46171158583c64113p-20),         \
        float_128(-0x1.3f9333294c89fb96239213f1e271p-23),        \
        float_128(0x1.85a501d5d74add9951dd571906a1p-28),         \
    }

#include "gamma_inc.h"
