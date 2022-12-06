/*	$OpenBSD: math_private.h,v 1.17 2014/06/02 19:31:17 kettenis Exp $	*/
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * from: @(#)fdlibm.h 5.1 93/09/24
 */

#ifndef _MATH_PRIVATE_OPENBSD_H_
#define _MATH_PRIVATE_OPENBSD_H_

#ifdef _IEEE128_FLOAT

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__

typedef union
{
  long double value;
  struct {
    u_int32_t mswhi;
    u_int32_t mswlo;
    u_int32_t lswhi;
    u_int32_t lswlo;
  } parts32;
  struct {
    u_int64_t msw;
    u_int64_t lsw;
  } parts64;
} ieee_quad_shape_type;

#endif

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__

typedef union
{
  long double value;
  struct {
    u_int32_t lswlo;
    u_int32_t lswhi;
    u_int32_t mswlo;
    u_int32_t mswhi;
  } parts32;
  struct {
    u_int64_t lsw;
    u_int64_t msw;
  } parts64;
} ieee_quad_shape_type;

#endif

/* Get two 64 bit ints from a long double.  */

#define GET_LDOUBLE_WORDS64(ix0,ix1,d)				\
do {								\
  ieee_quad_shape_type qw_u;					\
  qw_u.value = (d);						\
  (ix0) = qw_u.parts64.msw;					\
  (ix1) = qw_u.parts64.lsw;					\
} while (0)

/* Set a long double from two 64 bit ints.  */

#define SET_LDOUBLE_WORDS64(d,ix0,ix1)				\
do {								\
  ieee_quad_shape_type qw_u;					\
  qw_u.parts64.msw = (ix0);					\
  qw_u.parts64.lsw = (ix1);					\
  (d) = qw_u.value;						\
} while (0)

/* Get the more significant 64 bits of a long double mantissa.  */

#define GET_LDOUBLE_MSW64(v,d)					\
do {								\
  ieee_quad_shape_type sh_u;					\
  sh_u.value = (d);						\
  (v) = sh_u.parts64.msw;					\
} while (0)

/* Set the more significant 64 bits of a long double mantissa from an int.  */

#define SET_LDOUBLE_MSW64(d,v)					\
do {								\
  ieee_quad_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts64.msw = (v);					\
  (d) = sh_u.value;						\
} while (0)

/* Get the least significant 64 bits of a long double mantissa.  */

#define GET_LDOUBLE_LSW64(v,d)					\
do {								\
  ieee_quad_shape_type sh_u;					\
  sh_u.value = (d);						\
  (v) = sh_u.parts64.lsw;					\
} while (0)

static ALWAYS_INLINE int
issignalingl(long double x)
{
    u_int64_t high;
    if (!isnanl(x))
        return 0;
    GET_LDOUBLE_MSW64(high, x);
    if (!IEEE_754_2008_SNAN)
        return (high & 0x0000800000000000ULL) != 0;
    else
        return (high & 0x0000800000000000ULL) == 0;
}

static ALWAYS_INLINE int
__signbitl(long double x)
{
    int64_t high;
    GET_LDOUBLE_MSW64(high, x);
    return high < 0;
}

/* 128-bit long double */

union IEEEl2bits {
	long double	e;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t	sign	:1;
		uint64_t	exp	:15;
		uint64_t	manh	:48;
		uint64_t	manl	:64;
#endif
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
		uint64_t	manl	:64;
		uint64_t	manh	:48;
		uint64_t	exp	:15;
		uint64_t	sign	:1;
#endif
	} bits;
	/* TODO andrew: Check the packing here */
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t	expsign	:16;
		uint64_t	manh	:48;
		uint64_t	manl	:64;
#endif
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
		uint64_t	manl	:64;
		uint64_t	manh	:48;
		uint64_t	expsign	:16;
#endif
	} xbits;
};

#define	LDBL_NBIT	0
#define	LDBL_IMPLICIT_NBIT
#define	mask_nbit_l(u)	((void)0)
#define LDBL_INF_NAN_EXP    32767
#define LDBL_EXP_MASK           0x7fff
#define LDBL_EXP_SIGN           0x8000

#define	LDBL_MANH_SIZE	48
#define	LDBL_MANL_SIZE	64

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)((u).bits.manl >> 32);	\
	(a)[2] = (uint32_t)(u).bits.manh;		\
	(a)[3] = (uint32_t)((u).bits.manh >> 32);	\
} while(0)

#endif /* _IEEE128_FLOAT */

#ifdef _INTEL80_FLOAT

/* A union which permits us to convert between a long double and
   three 32 bit ints.  */

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__

typedef union
{
  long double value;
  struct {
#ifdef __LP64__
    int padh:32;
#endif
    int exp:16;
    int padl:16;
    u_int32_t msw;
    u_int32_t lsw;
  } parts;
} ieee_extended_shape_type;

#endif

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__

typedef union
{
  long double value;
  struct {
    u_int32_t lsw;
    u_int32_t msw;
    int exp:16;
    int padl:16;
#ifdef __LP64__
    int padh:32;
#endif
  } parts;
} ieee_extended_shape_type;

#endif

/* Get three 32 bit ints from a double.  */

#define GET_LDOUBLE_WORDS(se,ix0,ix1,d)				\
do {								\
  ieee_extended_shape_type ew_u;				\
  ew_u.value = (d);						\
  (se) = ew_u.parts.exp;					\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

/* Set a double from two 32 bit ints.  */

#define SET_LDOUBLE_WORDS(d,se,ix0,ix1)				\
do {								\
  ieee_extended_shape_type iw_u;				\
  iw_u.parts.exp = (se);					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

/* Get the more significant 32 bits of a long double mantissa.  */

#define GET_LDOUBLE_MSW(v,d)					\
do {								\
  ieee_extended_shape_type sh_u;				\
  sh_u.value = (d);						\
  (v) = sh_u.parts.msw;						\
} while (0)

/* Set the more significant 32 bits of a long double mantissa from an int.  */

#define SET_LDOUBLE_MSW(d,v)					\
do {								\
  ieee_extended_shape_type sh_u;				\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

/* Get int from the exponent of a long double.  */

#define GET_LDOUBLE_EXP(se,d)					\
do {								\
  ieee_extended_shape_type ge_u;				\
  ge_u.value = (d);						\
  (se) = ge_u.parts.exp;					\
} while (0)

/* Set exponent of a long double from an int.  */

#define SET_LDOUBLE_EXP(d,se)					\
do {								\
  ieee_extended_shape_type se_u;				\
  se_u.value = (d);						\
  se_u.parts.exp = (se);					\
  (d) = se_u.value;						\
} while (0)

static ALWAYS_INLINE int
issignalingl(long double x)
{
    u_int32_t high;
    if (!isnanl(x))
        return 0;
    GET_LDOUBLE_MSW(high, x);
    return (high & 0x40000000U) == 0;
}

static ALWAYS_INLINE int
__signbitl(long double x)
{
    int exp;
    GET_LDOUBLE_EXP(exp, x);
    return (exp & 0x8000) != 0;
}

union IEEEl2bits {
	long double	e;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t	sign	:1;
		uint64_t	exp	:15;
		uint64_t	manh	:32;
		uint64_t	manl	:32;
		uint64_t	junk	:16;
#endif
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
		uint64_t	manl	:32;
		uint64_t	manh	:32;
		uint64_t	exp	:15;
		uint64_t	sign	:1;
		uint64_t	junk	:16;
#endif
	} bits;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t 	expsign	:16;
		uint64_t        man	:64;
		uint64_t	junk	:16;
#endif
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
		uint64_t        man	:64;
		uint64_t 	expsign	:16;
		uint64_t	junk	:16;
#endif
	} xbits;
};

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)
#define LDBL_INF_NAN_EXP    32767
#define LDBL_EXP_MASK           0x7fff
#define LDBL_EXP_SIGN           0x8000

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)(u).bits.manh;		\
} while (0)

#endif /* _INTEL80_FLOAT */

#ifdef _DOUBLE_DOUBLE_FLOAT

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__

typedef union
{
  long double value;
  struct {
    double      msd;
    double      lsd;
  } doubles;
  struct {
    u_int32_t mswhi;
    u_int32_t mswlo;
    u_int32_t lswhi;
    u_int32_t lswlo;
  } parts32;
  struct {
    u_int64_t msw;
    u_int64_t lsw;
  } parts64;
  struct {
    int exp:12;
    u_int64_t   msw :52;
    int expl:12;
    u_int64_t   lsw :52;
  } parts;
} double_double_shape_type;

#endif /* __ORDER_BIG_ENDIAN__ */

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__

typedef union
{
  long double value;
  struct {
    double      msd;
    double      lsd;
  } doubles;
  struct {
    u_int32_t mswlo;
    u_int32_t mswhi;
    u_int32_t lswlo;
    u_int32_t lswhi;
  } parts32;
  struct {
    u_int64_t msw;
    u_int64_t lsw;
  } parts64;
  struct {
    u_int64_t   msw :52;
    int exp:12;
    u_int64_t   lsw :52;
    int expl:12;
  } parts;
} double_double_shape_type;

#endif /* __ORDER_LITTLE_ENDIAN__ */

/* Get two 64 bit ints from a long double.  */

#define GET_LDOUBLE_WORDS64(ix0,ix1,d)				\
do {								\
  double_double_shape_type qw_u;				\
  qw_u.value = (d);						\
  (ix0) = qw_u.parts64.msw;					\
  (ix1) = qw_u.parts64.lsw;					\
} while (0)

/* Set a long double from two 64 bit ints.  */

#define SET_LDOUBLE_WORDS64(d,ix0,ix1)				\
do {								\
  double_double_shape_type qw_u;				\
  qw_u.parts64.msw = (ix0);					\
  qw_u.parts64.lsw = (ix1);					\
  (d) = qw_u.value;						\
} while (0)

/* Get the more significant 64 bits of a long double mantissa.  */

#define GET_LDOUBLE_MSW64(v,d)					\
do {								\
  double_double_shape_type sh_u;				\
  sh_u.value = (d);						\
  (v) = sh_u.parts64.msw;					\
} while (0)

/* Set the more significant 64 bits of a long double mantissa from an int.  */

#define SET_LDOUBLE_MSW64(d,v)					\
do {								\
  double_double_shape_type sh_u;				\
  sh_u.value = (d);						\
  sh_u.parts64.msw = (v);					\
  (d) = sh_u.value;						\
} while (0)

/* Get the least significant 64 bits of a long double mantissa.  */

#define GET_LDOUBLE_LSW64(v,d)					\
do {								\
  double_double_shape_type sh_u;				\
  sh_u.value = (d);						\
  (v) = sh_u.parts64.lsw;					\
} while (0)

/* Get int from the exponent of a long double.  */

#define GET_LDOUBLE_EXP(se,d)					\
do {								\
  double_double_shape_type ge_u;				\
  ge_u.value = (d);						\
  (se) = ge_u.parts.exp;					\
} while (0)

/* Set exponent of a long double from an int.  */

#define SET_LDOUBLE_EXP(d,se)					\
do {								\
  double_double_shape_type se_u;				\
  se_u.value = (d);						\
  se_u.parts.exp = (se);					\
  (d) = se_u.value;						\
} while (0)

static ALWAYS_INLINE int
issignalingl(long double x)
{
    u_int64_t high;
    if (!isnanl(x))
        return 0;
    GET_LDOUBLE_MSW64(high, x);
    if (!IEEE_754_2008_SNAN)
        return (high & 0x0008000000000000ULL) != 0;
    else
        return (high & 0x0008000000000000ULL) == 0;
}

static ALWAYS_INLINE int
__signbitl(long double x)
{
    int exp;
    GET_LDOUBLE_EXP(exp, x);
    return exp < 0;
}

union IEEEl2bits {
	long double	e;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		u_int64_t	sign	:1;
		u_int64_t	exp	:11;
		u_int64_t       manh	:52;
                u_int64_t       signl   :1;
                u_int64_t       expl    :11;
                u_int64_t       manl    :52;
#endif
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
		u_int64_t	manh	:52;
		u_int64_t	exp	:11;
		u_int64_t	sign	:1;
                u_int64_t       manl    :52;
                u_int64_t       expl    :11;
                u_int64_t       signl   :1;
#endif
	} bits;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		u_int64_t 	expsign	:12;
		u_int64_t       manh	:52;
                u_int64_t       expsignl:12;
                u_int64_t       manl    :52;
#endif
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
		u_int64_t       manh	:52;
		u_int64_t 	expsign	:12;
                u_int64_t       manl    :52;
                u_int64_t       expsignl:12;
#endif
	} xbits;
        struct {
                double  dh;
                double  dl;
        } dbits;
};

#define	LDBL_NBIT	        0
#define	LDBL_IMPLICIT_NBIT
#define	mask_nbit_l(u)	        ((void)0)
#define LDBL_INF_NAN_EXP        2047
#define LDBL_EXP_MASK           0x7ff
#define LDBL_EXP_SIGN           0x800

#define	LDBL_MANH_SIZE	52

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)((u).bits.manl >> 32);	\
	(a)[2] = (uint32_t)(u).bits.manh;		\
	(a)[3] = (uint32_t)((u).bits.manh >> 32);	\
} while(0)

#endif /* _DOUBLE_DOUBLE_FLOAT */

/*
 * Common routine to process the arguments to nan(), nanf(), and nanl().
 */
void __scan_nan(uint32_t *__words, int __num_words, const char *__s);

/*
 * Functions internal to the math package, yet not static.
 */
double __exp__D(double, double);
struct Double __log__D(double);
long double __p1evll(long double, const long double *, int);
long double __polevll(long double, const long double *, int);

#endif /* _MATH_PRIVATE_OPENBSD_H_ */
