#ifndef _MACHFASTMATH_H
#define _MACHFASTMATH_H

#if defined(__GNUC__) && __STDC__ - 0 > 0

#define __str1__(__x) #x
#define __str2__(__x) __str1__(__x)
#define __U_L_PREFIX__ __str2__(__USER_LABEL_PREFIX__)

__extension__ double atan2(double, double)
  __asm__(__U_L_PREFIX__ "_f_atan2");
__extension__ double exp(double)
  __asm__(__U_L_PREFIX__ "_f_exp");
__extension__ double frexp(double, int*)
  __asm__(__U_L_PREFIX__ "_f_frexp");
__extension__ double ldexp(double, int)
  __asm__(__U_L_PREFIX__ "_f_ldexp");
__extension__ double log(double)
  __asm__(__U_L_PREFIX__ "_f_log");
__extension__ double log10(double)
  __asm__(__U_L_PREFIX__ "_f_log10");
__extension__ double pow(double, double)
  __asm__(__U_L_PREFIX__ "_f_pow");
__extension__ double tan(double)
  __asm__(__U_L_PREFIX__ "_f_tan");

#if !defined(__STRICT_ANSI__) || defined(__cplusplus) || __STDC_VERSION__ >= 199901L

__extension__ float atan2f(float, float)
  __asm__(__U_L_PREFIX__ "_f_atan2f");
__extension__ float expf(float)
  __asm__(__U_L_PREFIX__ "_f_expf");
__extension__ float frexpf(float, int*)
  __asm__(__U_L_PREFIX__ "_f_frexpf");
__extension__ float ldexpf(float, int)
  __asm__(__U_L_PREFIX__ "_f_ldexpf");
__extension__ float logf(float)
  __asm__(__U_L_PREFIX__ "_f_logf");
__extension__ float log10f(float)
  __asm__(__U_L_PREFIX__ "_f_log10f");
__extension__ float powf(float, float)
  __asm__(__U_L_PREFIX__ "_f_powf");
__extension__ float tanf(float)
  __asm__(__U_L_PREFIX__ "_f_tanf");
#endif

#else

double EXFUN(_f_atan2,(double, double));
double EXFUN(_f_exp,(double));
double EXFUN(_f_frexp,(double, int*));
double EXFUN(_f_ldexp,(double, int));
double EXFUN(_f_log,(double));
double EXFUN(_f_log10,(double));
double EXFUN(_f_pow,(double, double));

float EXFUN(_f_atan2f,(float, float));
float EXFUN(_f_expf,(float));
float EXFUN(_f_frexpf,(float, int*));
float EXFUN(_f_ldexpf,(float, int));
float EXFUN(_f_logf,(float));
float EXFUN(_f_log10f,(float));
float EXFUN(_f_powf,(float, float));

#define atan2(__y,__x)	_f_atan2((__y),(__x))
#define exp(__x)	_f_exp(__x)
#define frexp(__x,__p)	_f_frexp((__x),(__p))
#define ldexp(__x,__e)	_f_ldexp((__x),(__e))
#define log(__x)	_f_log(__x)
#define log10(__x)	_f_log10(__x)
#define pow(__x,__y)	_f_pow((__x),(__y))

#ifndef __STRICT_ANSI__
#define atan2f(__y,__x)	_f_atan2f((__y),(__x))
#define expf(__x)	_f_expf(__x)
#define frexpf(__x,__p)	_f_frexpf((__x),(__p))
#define ldexpf(__x,__e)	_f_ldexpf((__x),(__e))
#define logf(__x)	_f_logf(__x)
#define log10f(__x)	_f_log10f(__x)
#define powf(__x,y)	_f_powf((__x),(__y))
#endif

#endif /* GCC */
#endif /* _MACHFASTMATH_H */
