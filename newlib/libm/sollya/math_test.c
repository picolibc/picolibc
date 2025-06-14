#include "math2.h"
#include <stdio.h>
#include <stdint.h>

uint32_t
asuint_32(float32_t a)
{
    union {
        uint32_t        u;
        float32_t       f;
    } u;

    u.f = a;
    return u.u;
}

uint32_t
ulps_32(float32_t a, float32_t b)
{
    uint32_t    ua = asuint_32(a);
    uint32_t    ub = asuint_32(b);

    if (ua > ub)
        return ua - ub;
    return ub - ua;
}

uint64_t
asuint_64(float64_t a)
{
    union {
        uint64_t        u;
        float64_t       f;
    } u;

    u.f = a;
    return u.u;
}

uint64_t
ulps_64(float64_t a, float64_t b)
{
    uint64_t    ua = asuint_64(a);
    uint64_t    ub = asuint_64(b);

    if (ua > ub)
        return ua - ub;
    return ub - ua;
}

typedef __uint128_t uint128_t;

uint128_t
asuint_80(float80_t a)
{
    union {
        uint128_t        u;
        float80_t       f;
    } u;

    u.u = 0;
    u.f = a;
    return u.u;
}

uint128_t
ulps_80(float80_t a, float80_t b)
{
    uint128_t    ua = asuint_80(a);
    uint128_t    ub = asuint_80(b);

    if (ua > ub)
        return ua - ub;
    return ub - ua;
}

int main(void)
{
    float       xf;
    double      xd;
    long double xl;
    uint32_t    mf = 0;
    uint64_t    md = 0;
    uint128_t   ml = 0;

    printf("%a -> %a\n", -0x1.c18caep+2, gamma_32(-0x1.c18caep+2));
    for (xf = 0x1p-4f; xf <= 35; xf += 0x1p-4f) {
        float   l = tgammaf(xf);
        float   m = gamma_32(xf);
        uint32_t        u = ulps_32(l, m);

        if (u > mf) {
            printf("32 %a -> %a, %a (%u ulps)\n", xf, l, m, u);
            mf = u;
        }
    }
    for (xd = 0x1p-4; xd <= 172; xd += 0x1p-4) {
        double   l = tgamma(xd);
        double   m = gamma_64(xd);
        uint64_t        u = ulps_64(l, m);

        if (u > md) {
            printf("64 %a -> %a, %a (%lu ulps)\n", xd, l, m, u);
            md = u;
        }
    }
    for (xl = 0x1p-4l; xl <= 1755; xl += 0x1p-4l) {
        long double   l = tgammal(xl);
        long double   m = gamma_80(xl);
        uint128_t        u = ulps_80(l, m);

        if (u > ml) {
            printf("80 %La -> %La, %La (%lu ulps)\n", xl, l, m, (uint64_t) u);
            ml = u;
        }
    }
}
