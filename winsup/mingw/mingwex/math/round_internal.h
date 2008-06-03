#ifndef _ROUND_INTERNAL_H
/*
 * round_internal.h
 *
 * $Id$
 *
 * Provides a generic implementation of the numerical rounding
 * algorithm, which is shared by all functions in the `round()',
 * `lround()' and `llround()' families.
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 *
 * This is free software.  You may redistribute and/or modify it as you
 * see fit, without restriction of copyright.
 *
 * This software is provided "as is", in the hope that it may be useful,
 * but WITHOUT WARRANTY OF ANY KIND, not even any implied warranty of
 * MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.  At no
 * time will the author accept any form of liability for any damages,
 * however caused, resulting from the use of this software.
 *
 */
#define _ROUND_INTERNAL_H

#include <math.h>
#include <fenv.h>

#define TYPE_PASTE( NAME, TYPE )    NAME##TYPE

#define INPUT_TYPE                  INPUT_TYPEDEF( FUNCTION )
#define INPUT_TYPEDEF( FUNCTION )   TYPE_PASTE( FUNCTION, _input_type )
/*
 * The types for the formal parameter, to each of the derived functions.
 */
#define round_input_type            double
#define roundf_input_type           float
#define roundl_input_type           long double

#define lround_input_type           double
#define lroundf_input_type          float
#define lroundl_input_type          long double

#define llround_input_type          double
#define llroundf_input_type         float
#define llroundl_input_type         long double

#define RETURN_TYPE                 RETURN_TYPEDEF( FUNCTION )
#define RETURN_TYPEDEF( FUNCTION )  TYPE_PASTE( FUNCTION, _return_type )
/*
 * The types for the return value, from each of the derived functions.
 */
#define round_return_type           double
#define roundf_return_type          float
#define roundl_return_type          long double

#define lround_return_type          long
#define lroundf_return_type         long
#define lroundl_return_type         long

#define llround_return_type         long long
#define llroundf_return_type        long long
#define llroundl_return_type        long long

#define MAX_RETURN_VALUE            RETURN_MAX( FUNCTION )
#define RETURN_MAX( FUNCTION )      TYPE_PASTE( FUNCTION, _return_max )
/*
 * The maximum values which may be returned by each of the derived functions
 * in the `lround' or the `llround' families.
 */
#define lround_return_max           LONG_MAX
#define lroundf_return_max          LONG_MAX
#define lroundl_return_max          LONG_MAX

#define llround_return_max          LLONG_MAX
#define llroundf_return_max         LLONG_MAX
#define llroundl_return_max         LLONG_MAX

#define MIN_RETURN_VALUE            RETURN_MIN( FUNCTION )
#define RETURN_MIN( FUNCTION )      TYPE_PASTE( FUNCTION, _return_min )
/*
 * The minimum values which may be returned by each of the derived functions
 * in the `lround' or the `llround' families.
 */
#define lround_return_min           LONG_MIN
#define lroundf_return_min          LONG_MIN
#define lroundl_return_min          LONG_MIN

#define llround_return_min          LLONG_MIN
#define llroundf_return_min         LLONG_MIN
#define llroundl_return_min         LLONG_MIN

#define REF_VALUE( VALUE )          REF_TYPE( FUNCTION, VALUE )
#define REF_TYPE( FUNC, VAL )       TYPE_PASTE( FUNC, _ref )( VAL )
/*
 * Macros for expressing constant values of the appropriate data type,
 * for use in each of the derived functions.
 */
#define round_ref( VALUE )          VALUE
#define lround_ref( VALUE )         VALUE
#define llround_ref( VALUE )        VALUE

#define roundl_ref( VALUE )         TYPE_PASTE( VALUE, L )
#define lroundl_ref( VALUE )        TYPE_PASTE( VALUE, L )
#define llroundl_ref( VALUE )       TYPE_PASTE( VALUE, L )

#define roundf_ref( VALUE )         TYPE_PASTE( VALUE, F )
#define lroundf_ref( VALUE )        TYPE_PASTE( VALUE, F )
#define llroundf_ref( VALUE )       TYPE_PASTE( VALUE, F )

static __inline__
INPUT_TYPE __attribute__(( always_inline )) round_internal( INPUT_TYPE x )
#define ROUND_MODES ( FE_TONEAREST | FE_UPWARD | FE_DOWNWARD | FE_TOWARDZERO )
{
  /* Generic helper function, for rounding of the input parameter value to
   * the nearest integer value.
   */
  INPUT_TYPE z;
  unsigned short saved_CW, tmp_required_CW;

  /* Rounding method suggested by Danny Smith <dannysmith@users.sf.net>
   *
   * Save the FPU control word state, set rounding mode TONEAREST, round the
   * input value, then restore the original FPU control word state.
   */
  __asm__( "fnstcw %0;" : "=m"( saved_CW ));
  tmp_required_CW = ( saved_CW & ~ROUND_MODES ) | FE_TONEAREST;
  __asm__( "fldcw %0;" :: "m"( tmp_required_CW ));
  __asm__( "frndint;" : "=t"( z ) : "0"( x ));
  __asm__( "fldcw %0;" :: "m"( saved_CW ));

  /* We now have a possible rounded value; unfortunately the FPU uses the
   * `round-to-even' rule for exact mid-way cases, where both C99 and POSIX
   * require us to always round away from zero, so we need to adjust those
   * mid-way cases which the FPU rounded in the wrong direction.
   *
   * Correction method suggested by Greg Chicares <gchicares@sbcglobal.net>
   */
  return x < REF_VALUE( 0.0 )
    ? /*
       * For negative input values, an incorrectly rounded value will be
       * exactly 0.5 greater than the original value; when we find such an
       * exact rounding offset, we must subtract an additional 1.0 from the
       * rounded result, otherwise we return the rounded result unchanged.
       */
      z - x == REF_VALUE( 0.5 ) ? z - REF_VALUE( 1.0 ) : z

    : /* For positive input values, an incorrectly rounded value will be
       * exactly 0.5 less than the original value; when we find such an exact
       * rounding offset, we must add an additional 1.0 to the rounded result,
       * otherwise we return the rounded result unchanged.
       */
      x - z == REF_VALUE( 0.5 ) ? z + REF_VALUE( 1.0 ) : z;
}

#endif /* !defined _ROUND_INTERNAL_H: $RCSfile$: end of file */
