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

#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <quadmath.h>

#define binary32 float
#define binary64 double
#define binary80 long double
#define binary128 __float128

#define decl(t) t p, r

#define range_reduce(x,p) do {                  \
	p = 0.5;                                \
	while (x >= 2) {                        \
            x -= 1;                             \
            p *= x;                             \
	}                                       \
	while (x < 1) {                         \
            p /= x;                             \
            x += 1;                             \
	}                                       \
    } while(0)

#define poly(t, x,c,rf,ma) do {                 \
        int i = sizeof(c) / sizeof(c[0]) - 1;   \
        r = c[i];                               \
        t y = x - 1;                            \
        while (--i >= 0)                        \
            r = ma(y, r, c[i]);                 \
    } while(0)

#define f(t,c,ma) decl(t); range_reduce(x,p); poly(t,x,c,r,ma); return (r * p) * 2;

#define N32 11
static const binary32 gamma32_coeffs[ /*13 + 1 */] = {
#if N32 == 11
    /* 11 2 ulps */
    0x1p0 ,
    -0x1.2788c8p-1 ,
    0x1.fa63c2p-1 ,
    -0x1.d07b72p-1 ,
    0x1.f51152p-1 ,
    -0x1.ecb6a4p-1 ,
    0x1.d2ba4ep-1 ,
    -0x1.86b984p-1 ,
    0x1.0a02b6p-1 ,
    -0x1.07ec86p-2 ,
    0x1.497f74p-4 ,
    -0x1.7dd862p-7 ,
#endif
#if N32 == 12
    /* 12 2 ulps */
    0x1p0 ,
    -0x1.2788cep-1 ,
    0x1.fa6516p-1 ,
    -0x1.d09602p-1 ,
    0x1.f61bf6p-1 ,
    -0x1.f2ce8ep-1 ,
    0x1.e92d82p-1 ,
    -0x1.bcf384p-1 ,
    0x1.6124bep-1 ,
    -0x1.c07f9p-2 ,
    0x1.9c4f1p-3 ,
    -0x1.dc78f8p-5 ,
    0x1.004a6p-7 ,
#endif
#if N32 == 13
    /* 13 2 ulps */
    0x1p0 ,
    -0x1.2788dp-1 ,
    0x1.fa658ep-1 ,
    -0x1.d0a054p-1 ,
    0x1.f690eep-1 ,
    -0x1.f5e33ep-1 ,
    0x1.f68704p-1 ,
    -0x1.e3c26cp-1 ,
    0x1.ae6eap-1 ,
    -0x1.49fca4p-1 ,
    0x1.918e0ep-2 ,
    -0x1.601f9ap-3 ,
    0x1.83be42p-5 ,
    -0x1.8dd95p-8 ,
#endif
#if N32 == 14
    /* 14 3 ulps */
    0x1p0 ,
    -0x1.2788dp-1 ,
    0x1.fa6598p-1 ,
    -0x1.d0a1f6p-1 ,
    0x1.f6ac52p-1 ,
    -0x1.f6ddccp-1 ,
    0x1.fc121ap-1 ,
    -0x1.f8886ep-1 ,
    0x1.e3d39ap-1 ,
    -0x1.a9a412p-1 ,
    0x1.401f2p-1 ,
    -0x1.7b737ap-2 ,
    0x1.42abd8p-3 ,
    -0x1.57d7b6p-5 ,
    0x1.554b3p-8 ,
#endif
#if N32 == 15
    /* 15 3 ulps */
    0x1p0 ,
    -0x1.2788dp-1 ,
    0x1.fa659ap-1 ,
    -0x1.d0a268p-1 ,
    0x1.f6b5e2p-1 ,
    -0x1.f74abep-1 ,
    0x1.ff0a12p-1 ,
    -0x1.031b44p0 ,
    0x1.07a87cp0 ,
    -0x1.05a3aap0 ,
    0x1.dc3c0ap-1 ,
    -0x1.6e77c8p-1 ,
    0x1.b64068p-2 ,
    -0x1.738294p-3 ,
    0x1.8755dep-5 ,
    -0x1.7df17p-8 ,
#endif
#if N32 == 16
    /* 16 */
    0x1p0 ,
    -0x1.2788dp-1 ,
    0x1.fa659ep-1 ,
    -0x1.d0a332p-1 ,
    0x1.f6c65p-1 ,
    -0x1.f809cp-1 ,
    0x1.0244aep0 ,
    -0x1.10d90ep0 ,
    0x1.38301ep0 ,
    -0x1.819262p0 ,
    0x1.d5846cp0 ,
    -0x1.f335a8p0 ,
    0x1.a582dep0 ,
    -0x1.0724aep0 ,
    0x1.c2b4b4p-2 ,
    -0x1.d5fe52p-4 ,
    0x1.c0a91p-7 ,
#endif
#if 0
    0x1p0 ,
    -0x1.2788dp-1 ,
    0x1.fa65a6p-1 ,
    -0x1.d0a592p-1 ,
    0x1.f70edp-1 ,
    -0x1.fcd3c6p-1 ,
    0x1.1c069ep0 ,
    -0x1.cf3448p0 ,
    0x1.4b94a6p2 ,
    -0x1.1350d6p4 ,
    0x1.88cb4p5 ,
    -0x1.bda728p6 ,
    0x1.8be344p7 ,
    -0x1.11c444p8 ,
    0x1.2472b6p8 ,
    -0x1.dbfec2p7 ,
    0x1.206a3ap7 ,
    -0x1.f543a8p5 ,
    0x1.250b8cp4 ,
    -0x1.988706p1 ,
    0x1.f85cb8p-3 ,
#endif
};

static const binary64 gamma64_coeffs[ ] = {
#if 0
    /* 22 0 ulps */
    0x1p0 ,
    -0x1.2788cfc6fb5ffp-1 ,
    0x1.fa658c23aff89p-1 ,
    -0x1.d0a118f2bb037p-1 ,
    0x1.f6a51044a2817p-1 ,
    -0x1.f6c80d37b4ea1p-1 ,
    0x1.fc7df183d87e2p-1 ,
    -0x1.fdf2d44dce931p-1 ,
    0x1.fefe467629512p-1 ,
    -0x1.ff432ab0a9ab7p-1 ,
    0x1.fe8b500ac3ccap-1 ,
    -0x1.fb01539b4f9c4p-1 ,
    0x1.f0112ce1a40fep-1 ,
    -0x1.d5a6921aea375p-1 ,
    0x1.a2988f050505ap-1 ,
    -0x1.53678b385a92ep-1 ,
    0x1.e24bd1cbcafafp-2 ,
    -0x1.20d8e669e13dbp-2 ,
    0x1.17ba2799711bcp-3 ,
    -0x1.a0ac3a2ed403ap-5 ,
    0x1.bceb7dd1a30a4p-7 ,
    -0x1.2db743407a5edp-9 ,
    0x1.852e24ccfe06p-13 ,
#endif
    0x1p0 ,
    -0x1.2788cfc6fb5ffp-1 ,
    0x1.fa658c23aff89p-1 ,
    -0x1.d0a118f2bb037p-1 ,
    0x1.f6a51044a2817p-1 ,
    -0x1.f6c80d37b4ea2p-1 ,
    0x1.fc7df183d8813p-1 ,
    -0x1.fdf2d44dced38p-1 ,
    0x1.fefe46762c689p-1 ,
    -0x1.ff432ab0c2d09p-1 ,
    0x1.fe8b500b54b7cp-1 ,
    -0x1.fb01539db819ap-1 ,
    0x1.f0112ce9666d3p-1 ,
    -0x1.d5a6922e2813fp-1 ,
    0x1.a2988f2a1162ap-1 ,
    -0x1.53678b6fe269ap-1 ,
    0x1.e24bd24cef707p-2 ,
    -0x1.20d8e6dd2d567p-2 ,
    0x1.17ba283475de8p-3 ,
    -0x1.a0ac3b5e68e8bp-5 ,
    0x1.bceb7f6a307bp-7 ,
    -0x1.2db744923dd1dp-9 ,
    0x1.852e26d23416p-13 ,
};

#ifdef binary80
#define N80 28
static const binary80 gamma80_coeffs[ /* 26 + 1 */ ] = {
#if N80 == 21
    /* 21 819 ulps */
    0x1.fffffffffffffcccp-1 ,
    -0x1.2788cfc6fb551e44p-1 ,
    0x1.fa658c23a94a50b4p-1 ,
    -0x1.d0a118f11117894ap-1 ,
    0x1.f6a5100c20f65858p-1 ,
    -0x1.f6c8089cab872228p-1 ,
    0x1.fc7db12f27aafefap-1 ,
    -0x1.fdf057dc13772ad2p-1 ,
    0x1.feec28d712b3c93cp-1 ,
    -0x1.fedf30fe96325b2ep-1 ,
    0x1.fce0fe547c005056p-1 ,
    -0x1.f5709c4a5523c43ep-1 ,
    0x1.e15a6d10d1f559eap-1 ,
    -0x1.b6b5f56cf0f55958p-1 ,
    0x1.6ec716c2a05b2772p-1 ,
    -0x1.0e76bb7e9ef4ad06p-1 ,
    0x1.51a246427544f62p-2 ,
    -0x1.557798c61e55d942p-3 ,
    0x1.09b5fbb660163782p-4 ,
    -0x1.2840f8d41204a352p-6 ,
    0x1.a306e9f0f439b3d2p-9 ,
    -0x1.196413ddc8475ff6p-12 ,
#endif
#if N80 == 22
    /* 22 128 ulps */
    0x1.ffffffffffffff74p-1 ,
    -0x1.2788cfc6fb5f3b9ap-1 ,
    0x1.fa658c23afb2656ap-1 ,
    -0x1.d0a118f2ae113f94p-1 ,
    0x1.f6a510434144e782p-1 ,
    -0x1.f6c80d1f6d0a3c72p-1 ,
    0x1.fc7df05c72927ac6p-1 ,
    -0x1.fdf2ca2152ff36c8p-1 ,
    0x1.fefe034145088e46p-1 ,
    -0x1.ff41d5fc8bcf8be6p-1 ,
    0x1.fe860b693ffa8154p-1 ,
    -0x1.faf0d83a7951ff36p-1 ,
    0x1.efe817e0f99e138cp-1 ,
    -0x1.d554a63160e33baap-1 ,
    0x1.a215cc40e5491c3ep-1 ,
    -0x1.52c108cd6dd48c36p-1 ,
    0x1.e0fc2ab7890c833ap-2 ,
    -0x1.1fd0c7365cbd68ccp-2 ,
    0x1.167cbb067c8291e6p-3 ,
    -0x1.9e79e1aa0f0d67cap-5 ,
    0x1.ba37c95b5eec82a2p-7 ,
    -0x1.2ba7e5f998887386p-9 ,
    0x1.823f96da4b69bfe4p-13 ,
#endif
#if N80 == 23
    /* 23 186 ulps */
    0x1.ffffffffffffffe8p-1 ,
    -0x1.2788cfc6fb612026p-1 ,
    0x1.fa658c23b1020f04p-1 ,
    -0x1.d0a118f30a836642p-1 ,
    0x1.f6a51050c337c4d6p-1 ,
    -0x1.f6c80e5587a1e176p-1 ,
    0x1.fc7e02fc9b8cb4ecp-1 ,
    -0x1.fdf39545bdb5f7d4p-1 ,
    0x1.ff0466d00afbf66cp-1 ,
    -0x1.ff68e97917a7095cp-1 ,
    0x1.ff3f6e2c79b11a5p-1 ,
    -0x1.fda570320241af36p-1 ,
    0x1.f7f35068d35cb516p-1 ,
    -0x1.e87e0423b9dafc46p-1 ,
    0x1.c6c68dd6233ce296p-1 ,
    -0x1.8b36c9bac0e4bec4p-1 ,
    0x1.360a90d592ce3328p-1 ,
    -0x1.a7d5e45b10b1e35ep-2 ,
    0x1.e6b44b603f4482e6p-3 ,
    -0x1.c32e8cea6114814ap-4 ,
    0x1.419e52da9f8a7a1ap-5 ,
    -0x1.48f08ed7d58241f4p-7 ,
    0x1.abdf4da3966d0c68p-10 ,
    -0x1.0914995ba05096fcp-13 ,
#endif
#if N80 == 26
    /* 26 764 ulps */
    0x1p0 ,
    -0x1.2788cfc6fb618eccp-1 ,
    0x1.fa658c23b156f596p-1 ,
    -0x1.d0a118f32479cac2p-1 ,
    0x1.f6a51054fc83c348p-1 ,
    -0x1.f6c80ec1dd6e905p-1 ,
    0x1.fc7e0a493242cafep-1 ,
    -0x1.fdf3ef0292fc230cp-1 ,
    0x1.ff0799f86c0f3482p-1 ,
    -0x1.ff7f41378ffeb52ap-1 ,
    0x1.ffb970f56be4570cp-1 ,
    -0x1.ffb7508253062668p-1 ,
    0x1.ff30816eaf2a99aep-1 ,
    -0x1.fd121a35a8dbe736p-1 ,
    0x1.f6a1ddea6ba299a6p-1 ,
    -0x1.e6a0cb2b00184b22p-1 ,
    0x1.c5b6102ccf48a3eap-1 ,
    -0x1.8d95ed86a75a90fcp-1 ,
    0x1.3e5d1d2f83f87078p-1 ,
    -0x1.c4584d4f639fea6ap-2 ,
    0x1.14b589fe6820dc8p-2 ,
    -0x1.1a87e633a6a1ef3ap-3 ,
    0x1.d0a654588186588ep-5 ,
    -0x1.25f50ecbfe2cc418p-6 ,
    0x1.0b765bbcd692316p-8 ,
    -0x1.3695c9aa63cd69bap-11 ,
    0x1.58f6f369c9c0cb98p-15 ,
#endif
#if N80 == 27
    /* 27 317 ulps */
    0x1p0 ,
    -0x1.2788cfc6fb618f32p-1 ,
    0x1.fa658c23b1576a2cp-1 ,
    -0x1.d0a118f324a933bap-1 ,
    0x1.f6a510550671a062p-1 ,
    -0x1.f6c80ec320f06808p-1 ,
    0x1.fc7e0a64b72d7984p-1 ,
    -0x1.fdf3f0ac9ad97ba2p-1 ,
    0x1.ff07ad16b011e35ep-1 ,
    -0x1.ff7fe95b90e1469ap-1 ,
    0x1.ffbdf7b56aa722fap-1 ,
    -0x1.ffd0317b254bdaf2p-1 ,
    0x1.ff9f0ae3fa89bb22p-1 ,
    -0x1.fea3214fa4a3ad9p-1 ,
    0x1.fb4ead5a5fc047dcp-1 ,
    -0x1.f22976ca3a80e998p-1 ,
    0x1.dd4cb97eecd10b16p-1 ,
    -0x1.b599c590bb556cd8p-1 ,
    0x1.7693366634634fa8p-1 ,
    -0x1.2349fa7d942bcc5p-1 ,
    0x1.903a53b27707c59p-2 ,
    -0x1.d8405dcec90fe88ap-3 ,
    0x1.d0720b3b78b6e02ep-4 ,
    -0x1.6fc1688418f1fedap-5 ,
    0x1.c03ef445c19bc83ap-7 ,
    -0x1.8936740b46903d22p-9 ,
    0x1.b8b67571010d5e4ep-12 ,
    -0x1.d90a5a96fa809a9ap-16 ,
#endif
#if N80 == 28
#if 0
    /* 28 0 ulps */
    0x1p0 ,
    -0x1.2788cfc6fb618f46p-1 ,
    0x1.fa658c23b1578254p-1 ,
    -0x1.d0a118f324b3aa3ep-1 ,
    0x1.f6a5105508c989f4p-1 ,
    -0x1.f6c80ec372ce898p-1 ,
    0x1.fc7e0a6c32c59dd8p-1 ,
    -0x1.fdf3f12950726a5ap-1 ,
    0x1.ff07b31fff7aec5p-1 ,
    -0x1.ff8022bd1f5e4c5ep-1 ,
    0x1.ffbfa41b1f23169ap-1 ,
    -0x1.ffda2eee92e49e58p-1 ,
    0x1.ffcf64bfcd9570b2p-1 ,
    -0x1.ff62de10b597098ap-1 ,
    0x1.fdc29892cb7372fap-1 ,
    -0x1.f8d549255c4fb5fep-1 ,
    0x1.ec6c411e28d18acep-1 ,
    -0x1.d238feabfbd9fd94p-1 ,
    0x1.a3c917ec4198b516p-1 ,
    -0x1.5ebf72606119ac92p-1 ,
    0x1.08e8df86e2d5a818p-1 ,
    -0x1.603ec8c6d9c5331cp-2 ,
    0x1.915e6fe35e338526p-3 ,
    -0x1.7cd05eb2f9a3fe0ap-4 ,
    0x1.22dd1493fdbbbe38p-5 ,
    -0x1.56219aba4dcfd28ap-7 ,
    0x1.21deb8c44fef15b6p-9 ,
    -0x1.3a1b921782393048p-12 ,
    0x1.465697960e465dfap-16 ,
#endif
    0x1p0 ,
    -0x1.2788cfc6fb618f46p-1 ,
    0x1.fa658c23b1578254p-1 ,
    -0x1.d0a118f324b3aa3ep-1 ,
    0x1.f6a5105508c989f4p-1 ,
    -0x1.f6c80ec372ce897ep-1 ,
    0x1.fc7e0a6c32c59d56p-1 ,
    -0x1.fdf3f12950725b54p-1 ,
    0x1.ff07b31fff79de44p-1 ,
    -0x1.ff8022bd1f515118p-1 ,
    0x1.ffbfa41b1eae45a4p-1 ,
    -0x1.ffda2eee8fc360cp-1 ,
    0x1.ffcf64bfbcc0d79ep-1 ,
    -0x1.ff62de106d363db4p-1 ,
    0x1.fdc29891cef8e1dp-1 ,
    -0x1.f8d549228ab04c22p-1 ,
    0x1.ec6c411783741f9ap-1 ,
    -0x1.d238fe9f01c49214p-1 ,
    0x1.a3c917d73dca8e5p-1 ,
    -0x1.5ebf72442f96e85ep-1 ,
    0x1.08e8df67ab7c87d4p-1 ,
    -0x1.603ec88e323cd66p-2 ,
    0x1.915e6f90031df2fap-3 ,
    -0x1.7cd05e51298c535ep-4 ,
    0x1.22dd143aa682d30ep-5 ,
    -0x1.56219a3fed3533aap-7 ,
    0x1.21deb84e1a3414f6p-9 ,
    -0x1.3a1b9187f3ac4048p-12 ,
    0x1.465696f13cd90dfap-16 ,
#endif
#if N80 == 29
    /* 29 0 ulps */
    0x1p0 ,
    -0x1.2788cfc6fb618f48p-1 ,
    0x1.fa658c23b1578584p-1 ,
    -0x1.d0a118f324b54e0cp-1 ,
    0x1.f6a510550934c67ap-1 ,
    -0x1.f6c80ec3832bdd6ep-1 ,
    0x1.fc7e0a6dda5f82cp-1 ,
    -0x1.fdf3f147a23c726ap-1 ,
    0x1.ff07b4bb95df25cep-1 ,
    -0x1.ff80337367947172p-1 ,
    0x1.ffc02c72d136a5ep-1 ,
    -0x1.ffdda898d0cba2dap-1 ,
    0x1.ffe1cc1e61cc32f2p-1 ,
    -0x1.ffb2dc0e18d36a0cp-1 ,
    0x1.fee26e621dddcec8p-1 ,
    -0x1.fc3430861d29da36p-1 ,
    0x1.f4e10aa53d4b950cp-1 ,
    -0x1.e405741a6cb9d1c6p-1 ,
    0x1.c33e1867ec3902dp-1 ,
    -0x1.8d62a643c972986ep-1 ,
    0x1.42bd17e20f7d92fep-1 ,
    -0x1.d78e735f28da83b8p-2 ,
    0x1.2e4017db74b50438p-2 ,
    -0x1.4b6ab14ab8686606p-3 ,
    0x1.2e5eba7ed3c29ac6p-4 ,
    -0x1.bc33252e71ae6ce2p-6 ,
    0x1.f6ca7efb10bbe5f4p-8 ,
    -0x1.9a523dd09e7be562p-10 ,
    0x1.acc71536dc8f9c32p-13 ,
    -0x1.ae230aa2e0952a44p-17 ,
#endif
#if N80 == 30
    /* 30 0 ulps */
    0x1p0 ,
    -0x1.2788cfc6fb618f4ap-1 ,
    0x1.fa658c23b15787c4p-1 ,
    -0x1.d0a118f324b64674p-1 ,
    0x1.f6a51055096e603p-1 ,
    -0x1.f6c80ec38b852b52p-1 ,
    0x1.fc7e0a6eadabe8d8p-1 ,
    -0x1.fdf3f156b86848c4p-1 ,
    0x1.ff07b58aec96a006p-1 ,
    -0x1.ff803c1275f49588p-1 ,
    0x1.ffc075265c5c125ap-1 ,
    -0x1.ffdf96d40c3a9e46p-1 ,
    0x1.ffec85eca31d9686p-1 ,
    -0x1.ffe41530cc79cc9ep-1 ,
    0x1.ff9e9694a1958a7p-1 ,
    -0x1.fe8f0dc4962acb48p-1 ,
    0x1.fb3b7d948f2376e8p-1 ,
    -0x1.f27f0c583f7e7a8p-1 ,
    0x1.df21be65fb0b3f1ap-1 ,
    -0x1.bad1e721d8e132aep-1 ,
    0x1.8135a64bb626960ap-1 ,
    -0x1.3401feb4da64c73cp-1 ,
    0x1.b9e10a3fc27a8cd6p-2 ,
    -0x1.158f76554e59b056p-2 ,
    0x1.29e32f7c3bbfded2p-3 ,
    -0x1.09d65b8b0ffa5ba8p-4 ,
    0x1.7de6da4dc9bbec78p-6 ,
    -0x1.a6c02a0348967708p-8 ,
    0x1.517d7ca14b8fe784p-10 ,
    -0x1.5921eefe46ef1f56p-13 ,
    0x1.52fe55b173cd036ap-17 ,
#endif
#if N80 == 44
    /* 44 */
    0x1p0 ,
    -0x1.2788cfc6fb618f4ap-1 ,
    0x1.fa658c23b157883cp-1 ,
    -0x1.d0a118f324b6d386p-1 ,
    0x1.f6a5105509b57592p-1 ,
    -0x1.f6c80ec3a0916944p-1 ,
    0x1.fc7e0a72ddd8122cp-1 ,
    -0x1.fdf3f1f12e554b3p-1 ,
    0x1.ff07c66f06a073fp-1 ,
    -0x1.ff81aeff5370c1c6p-1 ,
    0x1.ffda080ab3e594f8p-1 ,
    -0x1.00a8a5eea2aa2712p0 ,
    0x1.08a46474d8da706cp0 ,
    -0x1.57cacc2b2aec0bcp0 ,
    0x1.fa01f00653e982cep1 ,
    -0x1.6d02c43fa015fa6ep4 ,
    0x1.18762d5c23476df6p7 ,
    -0x1.82dabab62802dd5cp9 ,
    0x1.d4a32f78138f2148p11 ,
    -0x1.f2c0763eebb600f6p13 ,
    0x1.d3b31a97bed1cd7cp15 ,
    -0x1.83886bcaf507e07p17 ,
    0x1.1c6379f42efe97bep19 ,
    -0x1.7250fcef9bfe144p20 ,
    0x1.ac57e0a13c2db1bp21 ,
    -0x1.b86c2c6f8372b20ep22 ,
    0x1.929982a767e45c3ap23 ,
    -0x1.47100c1ae12efb56p24 ,
    0x1.d7c940f9a392b138p24 ,
    -0x1.2d9d2657c6c86edp25 ,
    0x1.5507f8758319b746p25 ,
    -0x1.53ea4ddd4990fc7cp25 ,
    0x1.296e40b6f1a4a2ep25 ,
    -0x1.c686ebcb446c40f8p24 ,
    0x1.2d376226e852a668p24 ,
    -0x1.5740c2f6d00a0e88p23 ,
    0x1.4c87a53f2670bbc2p22 ,
    -0x1.0dcf0f4e0b4e0bfcp21 ,
    0x1.675f21bd6336de7cp19 ,
    -0x1.7dd144ddcbe7ba04p17 ,
    0x1.35f82c2895205bp15 ,
    -0x1.6600bd728d37c80ap12 ,
    0x1.fdd2c0db4b2548dp8 ,
    -0x1.1d445bf44b3f4b28p4 ,
    -0x1.0edcd6fdab2cf984p-1 ,
#endif
};
#endif
#ifdef binary128
#define N128 50
static const binary128 gamma128_coeffs[ /* 44 + 1 */ ] = {
#if N128 == 28
    /* 28 */
    0x1.ffffffffffffffffff14862c0de4p-1 ,
    -0x1.2788cfc6fb618f437995e29906e6p-1 ,
    0x1.fa658c23b1578093ba817efb057fp-1 ,
    -0x1.d0a118f324b31d804ad741670566p-1 ,
    0x1.f6a5105508b06d6d708fa303f42ap-1 ,
    -0x1.f6c80ec36fee30993c634e3dde76p-1 ,
    0x1.fc7e0a6bf85bb029cf6477a933cp-1 ,
    -0x1.fdf3f125ef866fd027c6504c5b1fp-1 ,
    0x1.ff07b2fa27ff52dbbf09271fb33ep-1 ,
    -0x1.ff802173a239545eecb9f5d552f5p-1 ,
    0x1.ffbf9b3625806674c6f92a76d563p-1 ,
    -0x1.ffd9fd5ca466cd6b369c472c2407p-1 ,
    0x1.ffce83312d52560abfbee039abcbp-1 ,
    -0x1.ff5f9027de023dc57013ec16e402p-1 ,
    0x1.fdb84f755e9486fc388cd75a6777p-1 ,
    -0x1.f8ba94a37b1a079e015340730ad2p-1 ,
    0x1.ec323f0f59f06f5247484a46f169p-1 ,
    -0x1.d1cf7408115956f44cb7a63419c1p-1 ,
    0x1.a3285d01e1d0dcd4609417007f11p-1 ,
    -0x1.5df31f04a73c5d29f4029329a0b8p-1 ,
    0x1.081126b80a407d252714e8ef49bdp-1 ,
    -0x1.5ec77a6cf4059155ca515dd65d52p-2 ,
    0x1.8f4ab945e74ab67efd6a26cb1eb8p-3 ,
    -0x1.7a75407dae3d5b5f0c3154912e7fp-4 ,
    0x1.20c6cc7a36700035420eb174c757p-5 ,
    -0x1.5359bae71b4d9d92c6b402b3be95p-7 ,
    0x1.1f4022e82afb3d4f1f2b65c7edabp-9 ,
    -0x1.36ff915c4995202da0f89e9b5ffbp-12 ,
    0x1.42d77c35cf7db689ca3ef0d6648ap-16 ,
#endif
#if N128 == 35128
    /* 35 using prec 128 */
    0x1.ffffffffffffffffffffffbc75a6p-1 ,
    -0x1.2788cfc6fb618f49a379c840ea97p-1 ,
    0x1.fa658c23b1578776462a5a78e238p-1 ,
    -0x1.d0a118f324b62f5fa87aab219ef8p-1 ,
    0x1.f6a51055096b52cafcc14595823fp-1 ,
    -0x1.f6c80ec38b67652fbc86dbf9d5c6p-1 ,
    0x1.fc7e0a6eb3064a6840002dd12f17p-1 ,
    -0x1.fdf3f157b67d3a50200664344833p-1 ,
    0x1.ff07b5a1677d65410eff9c3717efp-1 ,
    -0x1.ff803d670c651fb67e8b2ff0ed82p-1 ,
    0x1.ffc0840894c7013bda62425d1b3ap-1 ,
    -0x1.ffe017e704249ab5ef2244640bfep-1 ,
    0x1.fff003da928b46d3be9b85bd5f19p-1 ,
    -0x1.fff7cb72a2e0c766e57ae39914eep-1 ,
    0x1.fffaa6b6c37c39496e4002961317p-1 ,
    -0x1.fff6dae9bb92bb4fb26e9f101931p-1 ,
    0x1.ffde9df0f33210be49431e67c0b1p-1 ,
    -0x1.ff80c014fb3bce8240505083107cp-1 ,
    0x1.fe50d8f840fef8fe9770eec4bf5bp-1 ,
    -0x1.fb0135e0d5400e0d2270cd2c82dbp-1 ,
    0x1.f30129cf7ad9982ed4d44f0a6a4ap-1 ,
    -0x1.e23aa91984bb8837ec9fefcd644p-1 ,
    0x1.c3afb549f2c4d86e961e4a7779a7p-1 ,
    -0x1.936d203b55610d182e9255e83112p-1 ,
    0x1.51598dc2dfed867607b5f55723c7p-1 ,
    -0x1.0329f8a9e0459569b3231304212bp-1 ,
    0x1.670e5b324b9b34a090405e9cf309p-2 ,
    -0x1.b85e8f7a1ef729fe25a26956d6b6p-3 ,
    0x1.d53a94931070aa58b6bf9c1bfe7ep-4 ,
    -0x1.a99ef091ab25dae954799dec7c8fp-5 ,
    0x1.40e8a3fa0fd1d9fced046cd7d50ap-6 ,
    -0x1.863c27ca814c827861cf4c3a9dcap-8 ,
    0x1.6edf45989af7ba10a79a82941545p-10 ,
    -0x1.f398a1b621bb19e84b0264d209e3p-13 ,
    0x1.b5d6078e1740ece3e6d1b129ccd5p-16 ,
    -0x1.7266374bab203ed06ff661e865f3p-20 ,
#endif
#if N128 == 35256
    /* 35 with prec 256 */
    0x1.ffffffffffffffffffffffbc7589p-1 ,
    -0x1.2788cfc6fb618f49a379c8410d32p-1 ,
    0x1.fa658c23b1578776462a5aa7e65ap-1 ,
    -0x1.d0a118f324b62f5fa87ac639a6fcp-1 ,
    0x1.f6a51055096b52cafcc997bf5e06p-1 ,
    -0x1.f6c80ec38b67652fbe1603a27e52p-1 ,
    0x1.fc7e0a6eb3064a6871cb060194bap-1 ,
    -0x1.fdf3f157b67d3a54818ccf32d9a6p-1 ,
    0x1.ff07b5a1677d65896926f79bf7ap-1 ,
    -0x1.ff803d670c652338e6b23b7360ecp-1 ,
    0x1.ffc0840894c7226c550d636be48dp-1 ,
    -0x1.ffe017e704258935a5cf10e75127p-1 ,
    0x1.fff003da92903403c3e4c883e578p-1 ,
    -0x1.fff7cb72a2f19b6d78da570537c1p-1 ,
    0x1.fffaa6b6c386208e97cbce2236bdp-1 ,
    -0x1.fff6dae9ba4b44994d067755ec08p-1 ,
    0x1.ffde9df0e882b0e0bca273141933p-1 ,
    -0x1.ff80c014c47821c6d2561127c63cp-1 ,
    0x1.fe50d8f76cc5fbb44bc587851d72p-1 ,
    -0x1.fb0135de3f131f1282a333643343p-1 ,
    0x1.f30129c8cc3eb8d44cc6eab7c8cdp-1 ,
    -0x1.e23aa90b04705b680cc06c435b31p-1 ,
    0x1.c3afb52f4cc9783c0928d19110bdp-1 ,
    -0x1.936d2011b2fc8e41a082dbe752afp-1 ,
    0x1.51598d8b81e8ae510f2713c3b418p-1 ,
    -0x1.0329f86b475d252c1a6a4cd8ba38p-1 ,
    0x1.670e5aba60673314501e392a18d8p-2 ,
    -0x1.b85e8eb8a1e0d592aafa496dd2fbp-3 ,
    0x1.d53a938e6d09e0772d3cf2cd127fp-4 ,
    -0x1.a99eef7035a69d2c687aa41d73cbp-5 ,
    0x1.40e8a2f5b12eaf1605f3ee0b5baep-6 ,
    -0x1.863c2658b4e3c4196e48bd377347p-8 ,
    0x1.6edf44099b7e9466b94cbd1457dep-10 ,
    -0x1.f3989f4f9ba55e492cbf6f91ff86p-13 ,
    0x1.b5d6053471d7d1a63336c185a0ep-16 ,
    -0x1.72663518fe626ae1621da7c3e2dbp-20 ,
#endif
#if N128 == 35512
    /* 35 with prec 512 */
    0x1.ffffffffffffffffffffffbc7583p-1 ,
    -0x1.2788cfc6fb618f49a379c840e8d9p-1 ,
    0x1.fa658c23b1578776462a5a774556p-1 ,
    -0x1.d0a118f324b62f5fa87aaa61802cp-1 ,
    0x1.f6a51055096b52cafcc111a9697ap-1 ,
    -0x1.f6c80ec38b67652fbc7dbb7fc11ep-1 ,
    0x1.fc7e0a6eb3064a683ee0e3f1157dp-1 ,
    -0x1.fdf3f157b67d3a500609a3943db7p-1 ,
    0x1.ff07b5a1677d653f43ad8956ae18p-1 ,
    -0x1.ff803d670c651f9d9af00ff25956p-1 ,
    0x1.ffc0840894c70026f1a78f4a3de2p-1 ,
    -0x1.ffe017e7042490e426d0fdd80d64p-1 ,
    0x1.fff003da928afd14a4936ea6d03dp-1 ,
    -0x1.fff7cb72a2def809da974556b581p-1 ,
    0x1.fffaa6b6c3729f456a1ccf48e3cap-1 ,
    -0x1.fff6dae9bb6769117aaadf9887cfp-1 ,
    0x1.ffde9df0f28ad3d1475ae41a6fbfp-1 ,
    -0x1.ff80c014f910d06315b399e038e9p-1 ,
    0x1.fe50d8f83aca00fb2b28d8b81e65p-1 ,
    -0x1.fb0135e0c5e12ae6f44e2399b4fap-1 ,
    0x1.f30129cf59dfea698716ed6b77dcp-1 ,
    -0x1.e23aa91947683cd11f1d411826c8p-1 ,
    0x1.c3afb5498fefe885145a0ec06c15p-1 ,
    -0x1.936d203acb8b7c3bfdc8d8a004a1p-1 ,
    0x1.51598dc239ffd017a55ce25767fap-1 ,
    -0x1.0329f8a93483ded956c6e0a742bp-1 ,
    0x1.670e5b311b8451c12198383ea8dap-2 ,
    -0x1.b85e8f7855f2e7034d18f1272c76p-3 ,
    0x1.d53a9490cf2fc2d2f957f6feb3fap-4 ,
    -0x1.a99ef08f4e9501c9262740da367bp-5 ,
    0x1.40e8a3f80c73f508a8968f8feab1p-6 ,
    -0x1.863c27c7c8a15d3f8f45e4b4084ap-8 ,
    0x1.6edf4595ccd66e9204301d630e9fp-10 ,
    -0x1.f398a1b1fd88fb87cdbc32a75debp-13 ,
    0x1.b5d6078a315c83589dbdd69747efp-16 ,
    -0x1.72663748278ded048686f34540c5p-20 ,
#endif
#if N128 == 40512
    /* 40 with prec 512 */
    0x1.fffffffffffffffffffffffffd6ep-1 ,
    -0x1.2788cfc6fb618f49a37c7edfc7acp-1 ,
    0x1.fa658c23b15787764ad14aa5fb2bp-1 ,
    -0x1.d0a118f324b62f62d81881b5a95p-1 ,
    0x1.f6a51055096b53f55cacf84ff45cp-1 ,
    -0x1.f6c80ec38b67a8cea9f3f2981f1cp-1 ,
    0x1.fc7e0a6eb310ad9e383f796cbd53p-1 ,
    -0x1.fdf3f157b7a350b7f90ca599a88p-1 ,
    0x1.ff07b5a17fef3e73c0325d9544dbp-1 ,
    -0x1.ff803d68a01bc4c97120873d77edp-1 ,
    0x1.ffc0841d4d7d28de56dac4f43cfp-1 ,
    -0x1.ffe018c3ecac8f7befe810047f15p-1 ,
    0x1.fff00b6fa94376be479bc28cc4e9p-1 ,
    -0x1.fff80312771fe3742066bd24b65dp-1 ,
    0x1.fffbff071062210e81cfb303f69ep-1 ,
    -0x1.fffdf12b76f59c8ca3534d3af28fp-1 ,
    0x1.fffea39f8a52328e019cccdc8c5ap-1 ,
    -0x1.fffd97bea2ed58bfcce3e937ba5ap-1 ,
    0x1.fff6f522d54d6a182581bd54c384p-1 ,
    -0x1.ffdc4d5031e1ca2c0a73d05fafcap-1 ,
    0x1.ff810f75e53e3df45400d2a16c8ep-1 ,
    -0x1.fe6f7a4677f2510dedcd6edb43cbp-1 ,
    0x1.fb9d3f7f29fb6247bbfdc99a9f84p-1 ,
    -0x1.f50b77839631949d484e812e613ep-1 ,
    0x1.e789ebda944c2e3441cc5a5dd8cdp-1 ,
    -0x1.cf04a3e40b88a3ed4028c8a676f4p-1 ,
    0x1.a7b8b1112dab72c5f21eafee7ad9p-1 ,
    -0x1.70316a63cdcbe8e1fdd1d0c457d7p-1 ,
    0x1.2b23454213e894f3a3971bae24ebp-1 ,
    -0x1.bf8789c78afe0f44855c92043f37p-2 ,
    0x1.2fa73b140e38bf0de5a85d04f471p-2 ,
    -0x1.70548cf548364e92173e67a56d42p-3 ,
    0x1.897679d6b8b1c80a190bac318e54p-4 ,
    -0x1.6c4844456bb9103dfa8a852dae6ap-5 ,
    0x1.1f03ecefcceb5d261674fd631b45p-6 ,
    -0x1.786224924b9554a81d7995921ff5p-8 ,
    0x1.8ee611673ac421bcd8f8c21832bcp-10 ,
    -0x1.47c7a4828e40ea15ca1bd69eb499p-12 ,
    0x1.876c1604f55a431204cb7cb61a34p-15 ,
    -0x1.2de0d6f3c8c71533c6cd10e80396p-18 ,
    0x1.c321c23fdc0d11e99603e0d25b91p-23 ,
#endif
#if N128 == 40
    /* 40 */
    0x1.fffffffffffffffffffffffffd6ep-1 ,
    -0x1.2788cfc6fb618f49a37c7edfc7acp-1 ,
    0x1.fa658c23b15787764ad14aa5fb2bp-1 ,
    -0x1.d0a118f324b62f62d81881b5a95p-1 ,
    0x1.f6a51055096b53f55cacf84ff44fp-1 ,
    -0x1.f6c80ec38b67a8cea9f3f29815acp-1 ,
    0x1.fc7e0a6eb310ad9e383f7969b2e4p-1 ,
    -0x1.fdf3f157b7a350b7f90ca503bc38p-1 ,
    0x1.ff07b5a17fef3e73c0324a02ba5ep-1 ,
    -0x1.ff803d68a01bc4c9711eab439723p-1 ,
    0x1.ffc0841d4d7d28de56b8522e06bcp-1 ,
    -0x1.ffe018c3ecac8f7bedf1d2b3a4bp-1 ,
    0x1.fff00b6fa94376be308e0160e367p-1 ,
    -0x1.fff80312771fe37341ffa67cc1adp-1 ,
    0x1.fffbff0710622107869aab7dc677p-1 ,
    -0x1.fffdf12b76f59c5d5bfa0e57ee6dp-1 ,
    0x1.fffea39f8a52317d9ccef8ce592bp-1 ,
    -0x1.fffd97bea2ed537e4fc2132c461bp-1 ,
    0x1.fff6f522d54d53b23ee355172551p-1 ,
    -0x1.ffdc4d5031e177662619b26d6cc3p-1 ,
    0x1.ff810f75e53d339a74ea528f9f03p-1 ,
    -0x1.fe6f7a4677ef6460b61e41616691p-1 ,
    0x1.fb9d3f7f29f42fa5572fcc09614dp-1 ,
    -0x1.f50b778396220b562af3b1195304p-1 ,
    0x1.e789ebda942ebec4e9ce353f4e5dp-1 ,
    -0x1.cf04a3e40b57aedcb30cc819d7b7p-1 ,
    0x1.a7b8b1112d64037356120a8f6f67p-1 ,
    -0x1.70316a63cd709a19156bb5eb75bcp-1 ,
    0x1.2b23454213829664dc0ea6481d93p-1 ,
    -0x1.bf8789c78a379ae9da4c4b70b4cp-2 ,
    0x1.2fa73b140d915fc3ffbc331c9eedp-2 ,
    -0x1.70548cf5474311b07e49744bdb58p-3 ,
    0x1.897679d6b783aa85375bfb8e43f3p-4 ,
    -0x1.6c4844456a7bbb137834138c3472p-5 ,
    0x1.1f03ecefcbd56a8457053e813948p-6 ,
    -0x1.786224924a06f5d0910b4b82a129p-8 ,
    0x1.8ee6116738fd47e45541a9406f57p-10 ,
    -0x1.47c7a4828cb3303d1ad79b289fa9p-12 ,
    0x1.876c1604f3663b52e315b4423b51p-15 ,
    -0x1.2de0d6f3c734e0392c639b9c456dp-18 ,
    0x1.c321c23fd99f67e12ccffffa9971p-23 ,
#endif
#if N128 == 44
    /* 44 ulp 2341366876832928353 */
    0x1.ffffffffffffffffffffffffffffp-1 ,
    -0x1.2788cfc6fb618f49a37c7f01f74ep-1 ,
    0x1.fa658c23b15787764ad19684bcfap-1 ,
    -0x1.d0a118f324b62f62d85bc7c9a677p-1 ,
    0x1.f6a51055096b53f57c8efbe39d9fp-1 ,
    -0x1.f6c80ec38b67a8d8088c5107a2bfp-1 ,
    0x1.fc7e0a6eb310af7c9bc3d31663cp-1 ,
    -0x1.fdf3f157b7a39584e8c7bdb1a54ap-1 ,
    0x1.ff07b5a17ff6b1f94801586f307p-1 ,
    -0x1.ff803d68a0bc7a1bbaf67267957cp-1 ,
    0x1.ffc0841d584a2280c7aba7cf9d8cp-1 ,
    -0x1.ffe018c483e49476bf97682bf2fap-1 ,
    0x1.fff00b76802c4289cace2439af35p-1 ,
    -0x1.fff80354d44d140f24f46c12bd37p-1 ,
    0x1.fffc0128a6efae94f85220929dc8p-1 ,
    -0x1.fffe0026f0fcb9569c7a675b5139p-1 ,
    0x1.fffefe63bf23954e915205747213p-1 ,
    -0x1.ffff74bd0f6f0e5e56e5a8c9704ep-1 ,
    0x1.ffff7fab03490adb8677e67d8b74p-1 ,
    -0x1.fffe9d61b7e392be2bdd019ccb23p-1 ,
    0x1.fffa58a87408f1ced804f084272bp-1 ,
    -0x1.ffe9ee69d7ef657cedd6108c358bp-1 ,
    0x1.ffb28ba52bd9b06c8d8bda2a8fd7p-1 ,
    -0x1.ff0c9f6640b24073fde7e107f1b6p-1 ,
    0x1.fd51fdfad80aba0878842f95e22p-1 ,
    -0x1.f934ffdce5bee3eb24374d62a582p-1 ,
    0x1.f0794f27024c554cae854f588f1ep-1 ,
    -0x1.dfeb884e52f74d2c254e6e4177d4p-1 ,
    0x1.c3e95c03f46fd1ad52e01c73a3d5p-1 ,
    -0x1.99a3cdd28fa349a46b971277e5f6p-1 ,
    0x1.60cd1b1c98f923006abd8f1f5595p-1 ,
    -0x1.1cd671093efe1890502929fafa01p-1 ,
    0x1.a9808ee6649488f97c1010830ffbp-2 ,
    -0x1.224d717bcffdaf4b647f78c15d33p-2 ,
    0x1.655dac28156ac2fca108a7608a8cp-3 ,
    -0x1.87f41106c7e4b4d97a0e694ea523p-4 ,
    0x1.7a00436505bbdd0d702990cced17p-5 ,
    -0x1.3bdf55e43bf8cc228983af36074p-6 ,
    0x1.c190bb9e0006d21602ce290a2484p-8 ,
    -0x1.0a9973fef9bccf9f31167d0d7875p-9 ,
    0x1.0000d2d8aba683c90b346c8a188bp-11 ,
    -0x1.7dff7c08aee334e03369c7da1994p-14 ,
    0x1.9f20f0617fd3868775ca65460019p-17 ,
    -0x1.240e3b57077345b1f06cffa27e3p-20 ,
    0x1.8f1c123bc5c795f86aee5fe7394p-25 ,
#endif
#if N128 == 50
    /* 50 */
    0x1p0 ,
    -0x1.2788cfc6fb618f49a37c7f0202a6p-1 ,
    0x1.fa658c23b15787764ad196a08943p-1 ,
    -0x1.d0a118f324b62f62d85be450d849p-1 ,
    0x1.f6a51055096b53f57c9ee773b59ep-1 ,
    -0x1.f6c80ec38b67a8d80e1ab8fa6a95p-1 ,
    0x1.fc7e0a6eb310af7dee704f09d476p-1 ,
    -0x1.fdf3f157b7a395bf459f015cb2c6p-1 ,
    0x1.ff07b5a17ff6b99202c7486cadb5p-1 ,
    -0x1.ff803d68a0bd3f8d2d7800b5fb8cp-1 ,
    0x1.ffc0841d585a2abde75ef5b72a5ep-1 ,
    -0x1.ffe018c484f47c938f494302ca6dp-1 ,
    0x1.fff00b768f1c665116b97f85f3dep-1 ,
    -0x1.fff8035584df070798c1a5259458p-1 ,
    0x1.fffc012f94cca805daa8e19d8542p-1 ,
    -0x1.fffe0062ab5173cd60867ce6480ep-1 ,
    0x1.ffff002110f798418ffa4b79e7ddp-1 ,
    -0x1.ffff8008ddb9158af2c76d629a39p-1 ,
    0x1.ffffbff079b3a356671841f86f92p-1 ,
    -0x1.ffffdf70d67a7323db8edbb3dd6bp-1 ,
    0x1.ffffec5271233146fa68c0c469acp-1 ,
    -0x1.ffffe2f09d30a441565bb36dc0cp-1 ,
    0x1.ffff9168263bc998a44387d7f708p-1 ,
    -0x1.fffe1e03efcf1ae367208e70ff94p-1 ,
    0x1.fff8728ad592bb8a672f7ec34615p-1 ,
    -0x1.ffe4b6997139df04450a290ee06cp-1 ,
    0x1.ffa726757fee92d78d0b76dff26cp-1 ,
    -0x1.fefa8e713109c04b3607cac3456p-1 ,
    0x1.fd46f2bce32f1d9ab8e21b9b349cp-1 ,
    -0x1.f967f3fc4b57b5c1f3d4c245ccddp-1 ,
    0x1.f176301dcbb42cf03297562644abp-1 ,
    -0x1.e2bf8b7276f617d654065cc45382p-1 ,
    0x1.ca28144522ab2a9e7bc0e6d2e11cp-1 ,
    -0x1.a5147ef254b51e7f76707764a218p-1 ,
    0x1.72b2905d51365e0bf191627401fdp-1 ,
    -0x1.351227d17aa105fc4bf2f4a1cb5p-1 ,
    0x1.e2ae628ca7b691498e0051354f3cp-2 ,
    -0x1.5d3a02c575bf5221088e977edf6fp-2 ,
    0x1.cf70ebfdb350e7f710d1ca6e52efp-3 ,
    -0x1.1731dee63733b46f0aa9f1527043p-3 ,
    0x1.2e5a2d0acf89c5edef8a7e0c9a2dp-4 ,
    -0x1.232d05287202ac051700e4f07426p-5 ,
    0x1.ece4ae60fab4ba6d44fc4ce1a4abp-7 ,
    -0x1.69ada0dea446b8898021b4ac2f3ap-8 ,
    0x1.c49c595bee5a64fc5304c6d3119ep-10 ,
    -0x1.d8dfddf18a189f3d639e2e11ad06p-12 ,
    0x1.90e2c3c23a94a90866b4287a58a7p-14 ,
    -0x1.08b50caa073e2e51f4e1d28e6798p-16 ,
    0x1.fe85dfa590a46171158583c64113p-20 ,
    -0x1.3f9333294c89fb96239213f1e271p-23 ,
    0x1.85a501d5d74add9951dd571906a1p-28 ,
#endif
#if N128 == 60
    /* 60 ulp 4977360814464399961 */
    0x1p0 ,
    -0x1.2788cfc6fb618f49a37c7f0202a6p-1 ,
    0x1.fa658c23b15787764ad196a089d5p-1 ,
    -0x1.d0a118f324b62f62d85be45258f5p-1 ,
    0x1.f6a51055096b53f57c9ee9175acdp-1 ,
    -0x1.f6c80ec38b67a8d80e1bbdef9b8cp-1 ,
    0x1.fc7e0a6eb310af7deedac1271323p-1 ,
    -0x1.fdf3f157b7a395bf6482c459fa79p-1 ,
    0x1.ff07b5a17ff6b998bcf2d999250fp-1 ,
    -0x1.ff803d68a0bd40b18aa719bfff2dp-1 ,
    0x1.ffc0841d585a528b7402a8e9fe72p-1 ,
    -0x1.ffe018c484f8edc2504908e22179p-1 ,
    0x1.fff00b768f8645c57a5314dd34cap-1 ,
    -0x1.fff803558d3638265b006dbde978p-1 ,
    0x1.fffc013024d1be11ad53ec55c92dp-1 ,
    -0x1.fffe006b11b6b91732918b73e512p-1 ,
    0x1.ffff008e6fb6d5f11ac9d83823ecp-1 ,
    -0x1.ffff84eba597dcc489a15b6454d9p-1 ,
    0x1.fffff15348736d54677a950c698ap-1 ,
    -0x1.0000cd8842d16755a5ae525d1197p0 ,
    0x1.0006e958c8d3c5b5ca0939fd8603p0 ,
    -0x1.0031ee8682819cc9162570f2ababp0 ,
    0x1.01436512aa48185b601865093897p0 ,
    -0x1.0762cfbd9f133a87d427d295c76dp0 ,
    0x1.271b43ccd351107f7bd6ae1c7135p0 ,
    -0x1.bbfd8af51c10cee7e07c83eb6039p0 ,
    0x1.0d955e6e19fabe699bbdf039ab17p2 ,
    -0x1.b9f8d41c3dcf012f3b6481fd6881p3 ,
    0x1.7d5a41a37cbc9263c8d0a013486bp5 ,
    -0x1.38fedbbb0cb2fbadf8ab698102dcp7 ,
    0x1.db7ed923d9335ac5d93bbcc6de3ap8 ,
    -0x1.4c19b25d005a2faf58fff9cf12fdp10 ,
    0x1.aa0586c8a584c29dfd462b264f3ep11 ,
    -0x1.f5e545f2ea32ad759668a5ff3479p12 ,
    0x1.0f92e55f7926a81ddce21e7494ep14 ,
    -0x1.0e0392c1f080a9b49a29f91dfb3dp15 ,
    0x1.ed4c592ccd26d391f130b76eefeep15 ,
    -0x1.9deb437fb836bcc3d622c3416e9ap16 ,
    0x1.3ee760f451e7697fc6c7605f8021p17 ,
    -0x1.c2ebe5a3686e978f7878c5d1cc16p17 ,
    0x1.244829da20ccd2c631c94922e04ep18 ,
    -0x1.5b0056eab3fb5e179824c73ca60dp18 ,
    0x1.78bd443efea7453675aaf5136174p18 ,
    -0x1.75661838e48f9ce0863fe58d63f3p18 ,
    0x1.5123a4113e89fb8c9a191449414cp18 ,
    -0x1.14992140fc8a0b7f078e3f5b3468p18 ,
    0x1.9b2886eb0cd68ef77261f91eb50ap17 ,
    -0x1.13d7e5e8a94f777d77af02a37b07p17 ,
    0x1.4ca8e27e067d7a4871ddd72b387fp16 ,
    -0x1.66b76d8b067bdd1c4adc5081696bp15 ,
    0x1.57b9c12a1fd9219bc6ae86450c2bp14 ,
    -0x1.2275c54f87eda76e93e7bd5b3c6p13 ,
    0x1.ace1609f269af2791de9e7dcfba6p11 ,
    -0x1.116514ea6f4d2903d3cdb7678939p10 ,
    0x1.287197ce9baa325c42c33bd6bf05p8 ,
    -0x1.0bf5a8ddb55bf4ade81060f4474p6 ,
    0x1.88d7dcea9105cb64e08e8fc48e21p3 ,
    -0x1.c09e01325cc38abdaa36fa05b382p0 ,
    0x1.765232bfeca5b597916ea33bf22bp-3 ,
    -0x1.95e84cae5a6d066aeaea1fe0583bp-7 ,
    0x1.ad3dfdf125806b14b3ee592c516p-12 ,
#endif
#if N128 == 70
    /* 70 */
    0x1p0 ,
    -0x1.2788cfc6fb618f49a37c7f0202a6p-1 ,
    0x1.fa658c23b15787764ad196a08a6fp-1 ,
    -0x1.d0a118f324b62f62d85be4548e7bp-1 ,
    0x1.f6a51055096b53f57c9eec6d918ep-1 ,
    -0x1.f6c80ec38b67a8d80e1e9a44cb9cp-1 ,
    0x1.fc7e0a6eb310af7df07527a61841p-1 ,
    -0x1.fdf3f157b7a395c007a2ed197e47p-1 ,
    0x1.ff07b5a17ff6b9c9458fd771e801p-1 ,
    -0x1.ff803d68a0bd4bece910febe8ac2p-1 ,
    0x1.ffc0841d585c674bcd3d43e3a373p-1 ,
    -0x1.ffe018c48549c0a6042937cb76b2p-1 ,
    0x1.fff00b7699bf3af9f655a0d4fd3ep-1 ,
    -0x1.fff80356a53f9c6b949ce2b1b8b6p-1 ,
    0x1.fffc0149cf5246021db7e362b7a9p-1 ,
    -0x1.fffe027437dda1553597e6cda912p-1 ,
    0x1.ffff24a562e8d666c2b07c2a0207p-1 ,
    -0x1.0000dbe5778925baf09fbc719febp0 ,
    0x1.000f27fc649bfab527fd472c1e6cp0 ,
    -0x1.00bba1c8e88e421fbac43d123f83p0 ,
    0x1.0810fbe17ba20aa6512069fe24ffp0 ,
    -0x1.4fd83cbb8e0423648d1b90735b6bp0 ,
    0x1.e4ed65dc51f33496679595c5e4c3p1 ,
    -0x1.79902d15589a2cd8370ba797dc68p4 ,
    0x1.4f097572724cb7a3dd1f9739ed48p7 ,
    -0x1.17fab5dce3399b118a7a5032460fp10 ,
    0x1.ada889ac75d9d504b967882e6149p12 ,
    -0x1.2e473f36b111681f7d10cf738f43p15 ,
    0x1.8692de1c3690aae481cc0b7fdfe5p17 ,
    -0x1.d03f381a22a6d5bc40fb8f6b3907p19 ,
    0x1.fc75fe412c05d86df8eeec9f7c73p21 ,
    -0x1.00f179a3bc8fe417b37e1364d4bfp24 ,
    0x1.dfe495c45ecdad942f4cff6e871ep25 ,
    -0x1.9e8f2591a86e7695926ccd010ce9p27 ,
    0x1.4b9be6f48578839266adba6e5632p29 ,
    -0x1.eba8b1558a775b529167a89e3283p30 ,
    0x1.52069d0db4cbd20799085cc41318p32 ,
    -0x1.af511645ef19cfafafeab438114ap33 ,
    0x1.fef031ac2fb3f965ec59b9ca9d9p34 ,
    -0x1.190b5e972f2a5523868dccf31766p36 ,
    0x1.1f2ec56775abbda673b3b8566364p37 ,
    -0x1.1097ac41ca2a09585d92a938b497p38 ,
    0x1.e0a94d6f4d769f98cf75b5c252b7p38 ,
    -0x1.8988a83cd6492737c718c347e5a9p39 ,
    0x1.2b19e693432c5cc27b343484650dp40 ,
    -0x1.a5d985e689d679353d9a5cbaa278p40 ,
    0x1.13d62f7d10b94dd4429d9b7f314ep41 ,
    -0x1.4e317050e85a64b7c2035b6734bp41 ,
    0x1.76bb4ab7d4ead3bb5c2290e95003p41 ,
    -0x1.84687409d7a6a0675667de75bde1p41 ,
    0x1.739898fb58bc2b8406bf29d3892cp41 ,
    -0x1.4796e964d80ac999a2ca20a8bb6ep41 ,
    0x1.09958b4a35b0881edbba361c4ba4p41 ,
    -0x1.8b1b4896e071acf9531be9fc8f0cp40 ,
    0x1.0cee386410109fcfd758bc352b08p40 ,
    -0x1.4df3a01517ed432256d643eb7b08p39 ,
    0x1.78e814cf83fb6046e26ea5213994p38 ,
    -0x1.80fb4cbf77e2c6d5c30ec85eb14bp37 ,
    0x1.62194bd9ab6cf7e02266705874e2p36 ,
    -0x1.238b41240b44a7ee95d386cf50cp35 ,
    0x1.aab1938e8c73fb0d4be8d7fab613p33 ,
    -0x1.1323d714f2a63576f136fe2ec4f8p32 ,
    0x1.355ce4c3f2f3d3f44f561ee04ce9p30 ,
    -0x1.2b43a4dbeb099b3f3867e6c5df27p28 ,
    0x1.e9b468ee615eda00573f96adc701p25 ,
    -0x1.4b516b1c38ee431326a291ae884ap23 ,
    0x1.675f3ce982f22ce847f101d0fdc7p20 ,
    -0x1.2a9fcba768d8b845567c04d79e5dp17 ,
    0x1.61e32e8ade73d28b6d8a6c17d76bp13 ,
    -0x1.068ef78abd0c00505403ae152b4p9 ,
    0x1.6560d4bfd53aef84700e1dcb0007p3 ,
#endif
#if N128 == 80
    /* 80 only the leading bit was right */
    0x1p0 ,
    -0x1.2788cfc6fb618f49a37c7f0202a6p-1 ,
    0x1.fa658c23b15787764ad196a08b2dp-1 ,
    -0x1.d0a118f324b62f62d85be4583878p-1 ,
    0x1.f6a51055096b53f57c9ef3d99e61p-1 ,
    -0x1.f6c80ec38b67a8d80e271bd6d024p-1 ,
    0x1.fc7e0a6eb310af7df6d01826b072p-1 ,
    -0x1.fdf3f157b7a395c3640fc9e4322fp-1 ,
    0x1.ff07b5a17ff6bb1d6857e4ae73f9p-1 ,
    -0x1.ff803d68a0bdb46b7d623873b5d4p-1 ,
    0x1.ffc0841d587617b1a97241e98724p-1 ,
    -0x1.ffe018c48a7566e4535aa608f7a8p-1 ,
    0x1.fff00b7777e48870c5ed980c6a82p-1 ,
    -0x1.fff8037633d7045a383e76fd5b23p-1 ,
    0x1.fffc052166aa50a68a191daef7cdp-1 ,
    -0x1.fffe6a2f1eb9f45e9da82bd6c02p-1 ,
    0x1.00045a3758cfed9cee573a5bbbb1p0 ,
    -0x1.00643d99b09c57075f1e1fd06635p0 ,
    0x1.07377c409a1a851d87c53a3127ddp0 ,
    -0x1.767b55b7d515fd7fb2da2cd819cfp0 ,
    0x1.f469ab95dc2c379af759598a4466p2 ,
    -0x1.6e529b00583129a425f063c843a7p6 ,
    0x1.1094105c2fa10e9c22b3f552b957p10 ,
    -0x1.7404ecc69ca08e65b4cb8a7216d8p13 ,
    0x1.cf4464d9c32d2deab8e6d8f8f782p16 ,
    -0x1.07cd3ff3bccbbe1e867eeff4355dp20 ,
    0x1.1381ed7143f0eae8c94629bbb7fcp23 ,
    -0x1.088472df9a3ef2a35899ae54ac78p26 ,
    0x1.d403f7fa841cafe9ab5e304a3628p28 ,
    -0x1.7e4a8b70f482417b28205a5c78d7p31 ,
    0x1.20dfe3325805f380593085cd3554p34 ,
    -0x1.948fedfc1457c0f8eb11df0db537p36 ,
    0x1.06eeebc20e14d54de8360728df9ep39 ,
    -0x1.3daaa08b472843f20c8cc94079afp41 ,
    0x1.65302c78b6f9e012b8f9a7b78385p43 ,
    -0x1.7639af70bf7f5a79b60a98275d61p45 ,
    0x1.6db6819985d88d2563c557c8410cp47 ,
    -0x1.4dae5e80b9e806713ef4ed95e49ap49 ,
    0x1.1c7edfc0cb9c2012b2aff5e2cc59p51 ,
    -0x1.c5a82762df34abc38a0782b7a0f6p52 ,
    0x1.52769f246a37dbbf7309435be279p54 ,
    -0x1.d8dc282f051ad727fb190f5cf46dp55 ,
    0x1.35693bb140f3cc8d1125040d6d7fp57 ,
    -0x1.7b71fe59d0a40bf0ff7192cb016dp58 ,
    0x1.b42fde6895e8db55f210b4588455p59 ,
    -0x1.d61c2034eb42bb501229d62f0ce6p60 ,
    0x1.db1982722380e3106fbd5871f18ap61 ,
    -0x1.c23e1d65ee7634d9838fdbbb9a58p62 ,
    0x1.9018aff2823908998ab0250d7532p63 ,
    -0x1.4d556d7fd9de393f3fb37aac231fp64 ,
    0x1.044f3c68f377214f669269bf31d4p65 ,
    -0x1.7cf6f65ed01526bfa9281dd16874p65 ,
    0x1.051b1a86d5ba453247ad24770fcp66 ,
    -0x1.4f0d54af6f6a62474314074099e9p66 ,
    0x1.92371455f16e9296475b3e7a148cp66 ,
    -0x1.c35ad92b713ba5686a4099a937bp66 ,
    0x1.d909e479ddc3efb5d6039d4530dap66 ,
    -0x1.ce86914c1093c77e391f8602e662p66 ,
    0x1.a569caf5bbf723bedb08334e7b33p66 ,
    -0x1.6547c218be98f9414838cb702bdbp66 ,
    0x1.196a5230f833bd6747a5b1b1977p66 ,
    -0x1.9b1e2af4343165396e10b7c6e7f1p65 ,
    0x1.15e95f54662a5cc1e5120e6057ddp65 ,
    -0x1.5ae657907ce227da7968a04cbea5p64 ,
    0x1.8eb5ae92cb4c6d9680b91ca3a28ep63 ,
    -0x1.a4a69e38099b399520bd63a53e3bp62 ,
    0x1.95ee5dd586cb916aed1c820bbae6p61 ,
    -0x1.64d3cfe1cdb92b9ad7d7fa64bd86p60 ,
    0x1.1c59864087005125a0c51d71b267p59 ,
    -0x1.98855232c57836214f3b9fec4defp57 ,
    0x1.06c4709b845c25bde7e71aed8a8p56 ,
    -0x1.2c406afde210de57e08413e91a83p54 ,
    0x1.2dc16520c4be86e92f2fb44e26ddp52 ,
    -0x1.077dea4cab8a560884e2031706d5p50 ,
    0x1.899f7570f654d812059120dde5b4p47 ,
    -0x1.ecca8426735b743b39246f9d47d5p44 ,
    0x1.f6b561594ab3ab0cd2a4b3d20fbcp41 ,
    -0x1.912b6b3bbdef8ae37cb246fa8e94p38 ,
    0x1.d58fb8ba0f16c5d34da325473034p34 ,
    -0x1.6641373e6b1b36954a762cb3414fp30 ,
    0x1.0b352f8118b9d5f37f1a256e1301p25 ,
#endif
};
#endif

binary32 my_gamma32(binary32 x) { f(binary32, gamma32_coeffs, fmaf); }
binary64 my_gamma64(binary64 x) { f(binary64, gamma64_coeffs, fma); }
#ifdef binary80
binary80 my_gamma80(binary80 x) { f(binary80, gamma80_coeffs, fmal); }
#endif
#ifdef binary128
binary128 my_gamma128(binary128 x) { f(binary128, gamma128_coeffs, fmaq); }
#endif

#define gamma32 tgammaf
#define gamma64 tgamma
#define gamma80 tgammal
#define gamma128 tgammal

int32_t
asint32(float x)
{
    union { float f; int32_t i; } u;
    u.f = x;
    return u.i;
}

int64_t
asint64(float x)
{
    union { double f; int64_t i; } u;
    u.f = x;
    return u.i;
}

int64_t
ulp32(float x, float y)
{
    int32_t d = asint32(x) - asint32(y);
    if (d < 0) d = -d;
    return (int64_t) d;
}

int64_t
ulp64(double x, double y)
{
    int64_t d = asint64(x) - asint64(y);
    if (d < 0) d = -d;
    return (int64_t) d;
}

#ifdef binary80
__int128_t
asint80(binary80 x)
{
    union { binary80 f; __int128_t i; } u;
    u.i = 0;
    u.f = x;
    return u.i;
}

int64_t
ulp80(binary80 x, binary80 y)
{
    __int128_t d = asint80(x) - asint80(y);
    if (d < 0) d = -d;
    return (int64_t) d;
}
#endif

#ifdef binary128
__int128_t
asint128(binary128 x)
{
    union { binary128 f; __int128_t i; } u;
    u.i = 0;
    u.f = x;
    return u.i;
}

int64_t
ulp128(binary128 x, binary128 y)
{
    __int128_t d = asint128(x) - asint128(y);
    if (d < 0) d = -d;
    return (int64_t) d;
}
#endif

#if 1
#define MIN 0x1p-4
#define STEP 0x1p-4
#define MAX 2
#endif
#if 0
#define MIN 0x1p-10
#define MAX 64
#define STEP 0x1.00000010000001p-10
#endif

#define eval(t, lf, mf, uf, f) do {                                     \
        t l, m, x;                                                      \
        t mx = 0;                                                       \
        int64_t u, mu = 0;                                              \
        for (x = MIN; x <= MAX; x += (t) STEP) {                        \
            l = (t) lf(x);                                              \
            m = mf(x);                                                  \
            u = uf(l, m);                                               \
            if (u > mu) {                                               \
                mx = x; mu = u;                                         \
                printf("max error %" PRId64 " at " f " got " f " want " f "\n",  \
                       mu, mx, mf(mx), (t) lf(mx));                     \
            }                                                           \
        }                                                               \
    } while(0)


int main(void)
{
    eval(binary32, tgamma, my_gamma32, ulp32, "%.6a");
    eval(binary64, tgammal, my_gamma64, ulp64, "%.17a");
#ifdef binary80
    eval(binary80, tgammaq, my_gamma80, ulp80, "%.15La");
#endif
#ifdef binary128
    eval(binary128, tgammaq, my_gamma128, ulp128, "%.28Qa");
#endif
    return 0;
}
