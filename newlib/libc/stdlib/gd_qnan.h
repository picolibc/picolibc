#ifdef __IEEE_BIG_ENDIAN

#if !defined(__mips)
#define f_QNAN 0x7fc00000
#define d_QNAN0 0x7ff80000
#define d_QNAN1 0x0
#define ld_QNAN0 0x7ff80000
#define ld_QNAN1 0x0
#define ld_QNAN2 0x0
#define ld_QNAN3 0x0
#define ldus_QNAN0 0x7ff8
#define ldus_QNAN1 0x0
#define ldus_QNAN2 0x0
#define ldus_QNAN3 0x0
#define ldus_QNAN4 0x0
#elif defined(__mips_nan2008)
#define f_QNAN 0x7fc00000
#define d_QNAN0 0x7ff80000
#define d_QNAN1 0x0
#else
#define f_QNAN 0x7fbfffff
#define d_QNAN0 0x7ff7ffff
#define d_QNAN1 0xffffffff
#endif

#elif defined(__IEEE_LITTLE_ENDIAN)

#if !defined(__mips)
#define f_QNAN 0xffc00000
#define d_QNAN0 0x0
#define d_QNAN1 0xfff80000
#define ld_QNAN0 0x0
#define ld_QNAN1 0xc0000000
#define ld_QNAN2 0xffff
#define ld_QNAN3 0x0
#define ldus_QNAN0 0x0
#define ldus_QNAN1 0x0
#define ldus_QNAN2 0x0
#define ldus_QNAN3 0xc000
#define ldus_QNAN4 0xffff
#elif defined(__mips_nan2008)
#define f_QNAN 0x7fc00000
#define d_QNAN0 0x0
#define d_QNAN1 0x7ff80000
#else
#define f_QNAN 0x7fbfffff
#define d_QNAN0 0xffffffff
#define d_QNAN1 0x7ff7ffff
#endif

#else
#error IEEE endian not defined
#endif
