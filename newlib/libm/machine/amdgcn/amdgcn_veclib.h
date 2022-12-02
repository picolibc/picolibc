/*
 * Copyright 2023 Siemens
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/* Macro library used to help during conversion of scalar math functions to
   vectorized SIMD equivalents on AMD GCN.  */

/* Optimization at -O2 and above currently result in ICEs when converting
   between vector types.  */
#pragma GCC optimize ("O1")

typedef float v2sf __attribute__ ((vector_size (8)));
typedef float v4sf __attribute__ ((vector_size (16)));
typedef float v8sf __attribute__ ((vector_size (32)));
typedef float v16sf __attribute__ ((vector_size (64)));
typedef float v32sf __attribute__ ((vector_size (128)));
typedef float v64sf __attribute__ ((vector_size (256)));

typedef double v2df __attribute__ ((vector_size (16)));
typedef double v4df __attribute__ ((vector_size (32)));
typedef double v8df __attribute__ ((vector_size (64)));
typedef double v16df __attribute__ ((vector_size (128)));
typedef double v32df __attribute__ ((vector_size (256)));
typedef double v64df __attribute__ ((vector_size (512)));

typedef int v2si __attribute__ ((vector_size (8)));
typedef int v4si __attribute__ ((vector_size (16)));
typedef int v8si __attribute__ ((vector_size (32)));
typedef int v16si __attribute__ ((vector_size (64)));
typedef int v32si __attribute__ ((vector_size (128)));
typedef int v64si __attribute__ ((vector_size (256)));

typedef unsigned int v64usi __attribute__ ((vector_size (256)));

typedef long v2di __attribute__ ((vector_size (16)));
typedef long v4di __attribute__ ((vector_size (32)));
typedef long v8di __attribute__ ((vector_size (64)));
typedef long v16di __attribute__ ((vector_size (128)));
typedef long v32di __attribute__ ((vector_size (256)));
typedef long v64di __attribute__ ((vector_size (512)));

typedef union {
  v2sf t_v2sf;
  v4sf t_v4sf;
  v8sf t_v8sf;
  v16sf t_v16sf;
  v32sf t_v32sf;
  v64sf t_v64sf;

  v2df t_v2df;
  v4df t_v4df;
  v8df t_v8df;
  v16df t_v16df;
  v32df t_v32df;
  v64df t_v64df;

  v2si t_v2si;
  v4si t_v4si;
  v8si t_v8si;
  v16si t_v16si;
  v32si t_v32si;
  v64si t_v64si;

  v64usi t_v64usi;

  v2di t_v2di;
  v4di t_v4di;
  v8di t_v8di;
  v16di t_v16di;
  v32di t_v32di;
  v64di t_v64di;
} vector_union;

/* Cast between vectors with a different number of elements.  */

#define RESIZE_VECTOR(to_t, from) \
({ \
  __auto_type __from = (from); \
  *((to_t *) &__from); \
})

/* Bit-wise cast vector FROM to type TO_T.  */

#define CAST_VECTOR(to_t, from) \
({ \
  _Static_assert (sizeof (to_t) == sizeof (from)); \
  union { \
    typeof (from) __from; \
    to_t __to; \
  } __tmp; \
  __tmp.__from = (from); \
  __tmp.__to; \
})

#define NO_COND __mask

/* Note - __mask is _not_ accounted for in VECTOR_MERGE!  */
#define VECTOR_MERGE(vec1, vec2, cond) \
({ \
  _Static_assert (__builtin_types_compatible_p (typeof (vec1), typeof (vec2))); \
  union { \
    typeof (vec1) val; \
    v64si t_v64si; \
    v64di t_v64di; \
  } __vec1, __vec2, __res; \
  __vec1.val = (vec1); \
  __vec2.val = (vec2); \
  __builtin_choose_expr ( \
        sizeof (vec1) == sizeof (v64si), \
        ({ \
          v64si __bitmask = __builtin_convertvector ((cond), v64si); \
          __res.t_v64si = (__vec1.t_v64si & __bitmask) \
                          | (__vec2.t_v64si & ~__bitmask); \
        }), \
        ({ \
          v64di __bitmask = __builtin_convertvector ((cond), v64di); \
          __res.t_v64di = (__vec1.t_v64di & __bitmask) \
                          | (__vec2.t_v64di & ~__bitmask); \
        })); \
  __res.val; \
})

#define VECTOR_RETURN(retval, cond) \
do { \
  _Static_assert (__builtin_types_compatible_p (typeof (retval), typeof (__ret))); \
  __auto_type __cond = __builtin_convertvector ((cond), typeof (__mask)); \
  __auto_type __retval = (retval); \
  VECTOR_COND_MOVE (__ret, __retval, __cond); \
  __mask &= ~__cond; \
} while (0)

#define VECTOR_COND_MOVE(var, val, cond) \
do { \
  _Static_assert (__builtin_types_compatible_p (typeof (var), typeof (val))); \
  __auto_type __cond = __builtin_convertvector ((cond), typeof (__mask)); \
  var = VECTOR_MERGE ((val), var, __cond & __mask); \
} while (0)

#define VECTOR_IF(cond, cond_var) \
{ \
  __auto_type cond_var = (cond); \
  __auto_type __inv_cond = ~cond_var; \
  if (!ALL_ZEROES_P (cond_var)) \
  {

#define VECTOR_ELSEIF(cond, cond_var) \
  } \
  cond_var = __inv_cond & (cond); \
  __inv_cond &= ~(cond); \
  if (!ALL_ZEROES_P (cond_var)) \
  {

#define VECTOR_ELSE(cond_var) \
  } \
  cond_var = __inv_cond; \
  if (!ALL_ZEROES_P (cond_var)) \
  {

#define VECTOR_IF2(cond, cond_var, prev_cond_var) \
{ \
  __auto_type cond_var = (cond) & __builtin_convertvector (prev_cond_var, typeof (cond)); \
  __auto_type __inv_cond = ~(cond); \
  if (!ALL_ZEROES_P (cond_var)) \
  {

#define VECTOR_ELSEIF2(cond, cond_var, prev_cond_var) \
  } \
  cond_var = (cond) & __inv_cond & __builtin_convertvector (prev_cond_var, typeof (cond)); \
  __inv_cond &= ~(cond); \
  if (!ALL_ZEROES_P (cond_var)) \
  {

#define VECTOR_ELSE2(cond_var, prev_cond_var) \
  } \
  cond_var = __inv_cond & __builtin_convertvector (prev_cond_var, typeof (__inv_cond)); \
  if (!ALL_ZEROES_P (cond_var)) \
  {


#define VECTOR_ENDIF \
  } \
}

#define VECTOR_INIT_AUX(x, type) \
({ \
  typeof (x) __e = (x); \
  type __tmp = { \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e, \
    __e, __e, __e, __e, __e, __e, __e, __e }; \
  __tmp; \
})

#define VECTOR_INIT(x) \
  (_Generic ((x), int: VECTOR_INIT_AUX ((x), v64si), \
                  unsigned: VECTOR_INIT_AUX ((x), v64usi), \
                  long: VECTOR_INIT_AUX ((x), v64di), \
                  float: VECTOR_INIT_AUX ((x), v64sf), \
                  double: VECTOR_INIT_AUX ((x), v64df)))

#define VECTOR_WIDTH(TYPE) (sizeof (TYPE) / (V_SF_SI_P (TYPE) ? 4 : 8))

#define V_SF_SI_P(TYPE) \
  (__builtin_types_compatible_p (TYPE, v2sf) \
   || __builtin_types_compatible_p (TYPE, v4sf) \
   || __builtin_types_compatible_p (TYPE, v8sf) \
   || __builtin_types_compatible_p (TYPE, v16sf) \
   || __builtin_types_compatible_p (TYPE, v32sf) \
   || __builtin_types_compatible_p (TYPE, v64sf) \
   || __builtin_types_compatible_p (TYPE, v2si) \
   || __builtin_types_compatible_p (TYPE, v4si) \
   || __builtin_types_compatible_p (TYPE, v8si) \
   || __builtin_types_compatible_p (TYPE, v16si) \
   || __builtin_types_compatible_p (TYPE, v32si) \
   || __builtin_types_compatible_p (TYPE, v64si))

#define VECTOR_INIT_MASK(TYPE) \
({ \
  vector_union __mask; \
  __mask.t_v64di = VECTOR_INIT (0L); \
  for (int i = 0; i < VECTOR_WIDTH (TYPE); i++) \
    __mask.t_v64di[i] = -1; \
  __builtin_choose_expr (V_SF_SI_P (TYPE), __mask.t_v64si, __mask.t_v64di); \
})

#define ALL_ZEROES_P(x) (COND_TO_BITMASK(x) == 0)

#define COND_TO_BITMASK(x) \
({ \
  long __tmp = 0; \
  __auto_type __x = __builtin_convertvector((x), typeof (__mask)) & __mask; \
  __builtin_choose_expr (sizeof (__mask) == 256, \
                         ({ asm ("v_cmp_ne_u32_e64 %0, %1, 0" \
                                 : "=Sg" (__tmp) \
                                 : "v" (__x)); }), \
                         ({ asm ("v_cmp_ne_u64_e64 %0, %1, 0" \
                                 : "=Sg" (__tmp) \
                                 : "v" (__x)); })); \
  __tmp; \
})

#define VECTOR_WHILE(cond, cond_var, prev_cond_var) \
{ \
  __auto_type cond_var = prev_cond_var; \
  for (;;) { \
    cond_var &= (cond); \
    if (ALL_ZEROES_P (cond_var)) \
      break;

#define VECTOR_ENDWHILE \
  } \
}

#define DEF_VS_MATH_FUNC(rettype, name, args...) \
    rettype v64sf##_##name##_aux (args, v64si __mask)

#define DEF_VD_MATH_FUNC(rettype, name, args...) \
    rettype v64df##_##name##_aux (args, v64di __mask)

/* Use this for predicate functions that take a vector of doubles but
   return a vector of ints.  */
#define DEF_VD_MATH_PRED(rettype, name, args...) \
    rettype v64df##_##name##_aux (args, v64si __mask)

#define FUNCTION_INIT(rettype) \
  rettype __ret

#define FUNCTION_RETURN \
  return __ret

#define DEF_VARIANT(FUN, TRET, TARG, COUNT) \
v##COUNT##TRET \
v##COUNT##TARG##_##FUN (v##COUNT##TARG __arg) \
{ \
  __auto_type __upsized_arg = RESIZE_VECTOR (v64##TARG, __arg); \
  __auto_type __mask = VECTOR_INIT_MASK (v##COUNT##TRET); \
  __auto_type __result = v64##TARG##_##FUN##_aux (__upsized_arg, __mask); \
  return RESIZE_VECTOR (v##COUNT##TRET, __result); \
}

#define DEF_VARIANT2(FUN, TRET, TARG, COUNT) \
v##COUNT##TRET \
v##COUNT##TARG##_##FUN (v##COUNT##TARG __arg1, v##COUNT##TARG __arg2) \
{ \
  __auto_type __upsized_arg1 = RESIZE_VECTOR (v64##TARG, __arg1); \
  __auto_type __upsized_arg2 = RESIZE_VECTOR (v64##TARG, __arg2); \
  __auto_type __mask = VECTOR_INIT_MASK (v##COUNT##TRET); \
  __auto_type __result = v64##TARG##_##FUN##_aux (__upsized_arg1, __upsized_arg2, __mask); \
  return RESIZE_VECTOR (v##COUNT##TRET, __result); \
}

#define DEF_VARIANTS(FUN, RETTYPE, ARGTYPE) \
  DEF_VARIANT (FUN, RETTYPE, ARGTYPE, 2) \
  DEF_VARIANT (FUN, RETTYPE, ARGTYPE, 4) \
  DEF_VARIANT (FUN, RETTYPE, ARGTYPE, 8) \
  DEF_VARIANT (FUN, RETTYPE, ARGTYPE, 16) \
  DEF_VARIANT (FUN, RETTYPE, ARGTYPE, 32) \
  DEF_VARIANT (FUN, RETTYPE, ARGTYPE, 64)

#define DEF_VARIANTS2(FUN, RETTYPE, ARGTYPE) \
  DEF_VARIANT2 (FUN, RETTYPE, ARGTYPE, 2) \
  DEF_VARIANT2 (FUN, RETTYPE, ARGTYPE, 4) \
  DEF_VARIANT2 (FUN, RETTYPE, ARGTYPE, 8) \
  DEF_VARIANT2 (FUN, RETTYPE, ARGTYPE, 16) \
  DEF_VARIANT2 (FUN, RETTYPE, ARGTYPE, 32) \
  DEF_VARIANT2 (FUN, RETTYPE, ARGTYPE, 64)
