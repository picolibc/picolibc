#ifndef _MACHINE_IEEE_H_
#define _MACHINE_IEEE_H_
#include <sys/types.h>
#include <sys/cdefs.h>
#include <machine/ieeefp.h>
#include <float.h>

#if LDBL_MANT_DIG == 24
#define EXT_IMPLICIT_NBIT
#define EXT_TO_ARRAY32(p, a) do {      \
    (a)[0] = (p)->ext_frac;  \
} while (0)
#ifdef __IEEE_LITTLE_ENDIAN
struct ieee_ext {
    __uint32_t   ext_frac:23;
    __uint32_t   ext_exp:8;
    __uint32_t   ext_sign:1;
};
#endif
#ifdef __IEEE_BIG_ENDIAN
struct ieee_ext {
#ifndef ___IEEE_BYTES_LITTLE_ENDIAN
    __uint32_t   ext_sign:1;
    __uint32_t   ext_exp:8;
    __uint32_t   ext_frac:23;
#else /* ARMEL without __VFP_FP__ */
    __uint32_t   ext_frac:23;
    __uint32_t   ext_exp:8;
    __uint32_t   ext_sign:1;
#endif
};
#endif
#elif LDBL_MANT_DIG == 53
#define EXT_IMPLICIT_NBIT
#define EXT_TO_ARRAY32(p, a) do {       \
    (a)[0] = (p)->ext_fracl;  \
    (a)[1] = (p)->ext_frach;  \
} while (0)
#ifdef __IEEE_LITTLE_ENDIAN
struct ieee_ext {
    __uint32_t   ext_fracl;
    __uint32_t   ext_frach:20;
    __uint32_t   ext_exp:11;
    __uint32_t   ext_sign:1;
};
#endif
#ifdef __IEEE_BIG_ENDIAN
struct ieee_ext {
#ifndef ___IEEE_BYTES_LITTLE_ENDIAN
    __uint32_t   ext_sign:1;
    __uint32_t   ext_exp:11;
    __uint32_t   ext_frach:20;
#else /* ARMEL without __VFP_FP__ */
    __uint32_t   ext_frach:20;
    __uint32_t   ext_exp:11;
    __uint32_t   ext_sign:1;
#endif
    __uint32_t   ext_fracl;
};
#endif
#elif LDBL_MANT_DIG == 64
#define EXT_TO_ARRAY32(p, a) do {       \
    (a)[0] = (p)->ext_fracl;  \
    (a)[1] = (p)->ext_frach;  \
} while (0)
#ifdef __IEEE_LITTLE_ENDIAN /* for Intel CPU */
struct ieee_ext {
    __uint32_t   ext_fracl;
    __uint32_t   ext_frach;
    __uint32_t   ext_exp:15;
    __uint32_t   ext_sign:1;
    __uint32_t   ext_padl:16;
    __uint32_t   ext_padh;
};
#endif
#ifdef __IEEE_BIG_ENDIAN
struct ieee_ext {
#ifndef ___IEEE_BYTES_LITTLE_ENDIAN /* for m68k */
    __uint32_t   ext_sign:1;
    __uint32_t   ext_exp:15;
    __uint32_t   ext_pad:16;
#else /* ARM FPA10 math coprocessor */
    __uint32_t   ext_exp:15;
    __uint32_t   ext_pad:16;
    __uint32_t   ext_sign:1;
#endif
    __uint32_t   ext_frach;
    __uint32_t   ext_fracl;
};
#endif
#elif LDBL_MANT_DIG == 113
#define EXT_IMPLICIT_NBIT
#define EXT_TO_ARRAY32(p, a) do {        \
    (a)[0] = (p)->ext_fracl;   \
    (a)[1] = (p)->ext_fraclm;  \
    (a)[2] = (p)->ext_frachm;  \
    (a)[3] = (p)->ext_frach;   \
} while (0)
#ifdef __IEEE_LITTLE_ENDIAN
struct ieee_ext {
    __uint32_t   ext_fracl;
    __uint32_t   ext_fraclm;
    __uint32_t   ext_frachm;
    __uint32_t   ext_frach:16;
    __uint32_t   ext_exp:15;
    __uint32_t   ext_sign:1;
};
#endif
#ifdef __IEEE_BIG_ENDIAN
struct ieee_ext {
#ifndef ___IEEE_BYTES_LITTLE_ENDIAN
    __uint32_t   ext_sign:1;
    __uint32_t   ext_exp:15;
    __uint32_t   ext_frach:16;
#else /* ARMEL without __VFP_FP__ */
    __uint32_t   ext_frach:16;
    __uint32_t   ext_exp:15;
    __uint32_t   ext_sign:1;
#endif
    __uint32_t   ext_frachm;
    __uint32_t   ext_fraclm;
    __uint32_t   ext_fracl;
};
#endif
#endif

#endif /* _MACHINE_IEEE_H_ */
