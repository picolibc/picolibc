#include <math.h>
#include <stdio.h>
#include <inttypes.h>

#define N_SIN 12

/* 0..3/4 */
static const double sin_coeffs_lo[ N_SIN ] = {
    0x1.96c572de7c31ap-100 ,
    0x1p0 ,
    -0x1.d2c36ec769d57p-49 ,
    -0x1.55555555538b8p-3 ,
    -0x1.385b3b40c93b7p-38 ,
    0x1.1111112b73132p-7 ,
    -0x1.46e204174b4bp-32 ,
    -0x1.a01966033bdcfp-13 ,
    -0x1.768168286a0eep-29 ,
    0x1.726b4f90c06d9p-19 ,
    -0x1.f430a7432c234p-29 ,
    -0x1.93f86b29401e7p-26 ,
};

/* 3/4..pi/2 */
static const double sin_coeffs_hi[ N_SIN ] = {
    0x1.5cffc16bf8f0dp-1 ,
    0x1.769fec6552172p-1 ,
    -0x1.5cffc16bfb024p-2 ,
    -0x1.f37fe5dbbb0acp-4 ,
    0x1.d15501c565ba2p-6 ,
    0x1.8f998817cce95p-8 ,
    -0x1.f05b2330a6d17p-11 ,
    -0x1.306fa74d36fd5p-13 ,
    0x1.1b503a151bb97p-16 ,
    0x1.11d3cf9ddbcfep-19 ,
    -0x1.bbe5019152203p-23 ,
    -0x1.56b451b540879p-27 ,
};

double my_cos(double x);
double my_sin(double x);

double
my_sin(double x)
{
    unsigned i;
    const double *c;

    if (x < 0x1p-27) return x;

    if (x >= 0.75) {
        x -= 0.75;
        c = sin_coeffs_hi;
    } else {
        c = sin_coeffs_lo;
    }

    double r = c[N_SIN-1];
    for (i = N_SIN-2; ; i--) {
        r = x * r + c[i];
        if (i == 0)
            break;
    }
    return r;
}

static const double cos_coeffs[] = {
    -0x1.2867404367335p-73 ,
    0x1.9f6df7cc70fe2p-65 ,
    0x1p-1 ,
    -0x1.47f6c76fc0a5fp-51 ,
    -0x1.5555555554506p-5 ,
    -0x1.3a0513f39d1bbp-41 ,
    0x1.6c16c18446575p-10 ,
    -0x1.15c4ea47eac51p-35 ,
    -0x1.a01985ed83affp-16 ,
    -0x1.180ab6cae5f4fp-32 ,
    0x1.2848957e7643dp-22 ,
    -0x1.4e3868e1bc725p-32 ,
    -0x1.0e306e7907829p-29 ,
};

#define N_COS (sizeof(cos_coeffs)/sizeof(cos_coeffs[0]))

double
my_cos(double x)
{
    unsigned i;
    double r = 0;
    if (x >= M_PI/4) return my_sin(M_PI/2 - x);
    if (x < 0x1p-27) return 1;
    for (i = N_COS-1; ; i--) {
        r = x * r + cos_coeffs[i];
        if (i == 0)
            break;
    }
    return 1 - r;
}

int64_t
as_int(double x)
{
    union {double d; int64_t u; } u;
    u.d = x;
    return u.u;
}

int64_t
ulps(double a, double b)
{
    int64_t     ai = as_int(a);
    int64_t     bi = as_int(b);

    int64_t     d = ai > bi ? ai - bi : bi - ai;
    return d;
}

#if 1
#define FUNC sinl
#define MY_FUNC my_sin
#endif

#if 0
#define FUNC cos
#define MY_FUNC my_cos
#endif

#define MIN 0
#define MAX (M_PI/2)

int main(void)
{
    double      x;
    int64_t     maxerr = 0;
    int64_t     err;
    uint64_t    nerr = 0;
    uint64_t    ntotal = 0;
    double      errval = 0;

    for (x = MIN; x < MAX; x += 0x1p-20) {
        double libm = FUNC(x);
        double my = MY_FUNC(x);

        err = ulps(libm, my);
        if (err)
            nerr++;
        if (err > maxerr) {
            maxerr = err;
            errval = x;
        }
        ntotal++;
    }
    printf("max ulps %" PRId64 " at %.17f\n", maxerr, errval);
    printf("%" PRIu64 " values with error out of %" PRIu64 "(%f %%)\n", nerr, ntotal, (double) nerr * 100 / (double) ntotal);
    return 0;
}
