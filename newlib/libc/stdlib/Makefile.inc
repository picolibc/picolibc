libc_a_SOURCES += \
	%D%/__adjust.c \
	%D%/__atexit.c \
	%D%/__call_atexit.c \
	%D%/__exp10.c \
	%D%/__ten_mu.c \
	%D%/_Exit.c \
	%D%/abort.c \
	%D%/abs.c \
	%D%/aligned_alloc.c \
	%D%/assert.c \
	%D%/atexit.c \
	%D%/atof.c \
	%D%/atoff.c \
	%D%/atoi.c \
	%D%/atol.c \
	%D%/calloc.c \
	%D%/callocr.c \
	%D%/cfreer.c \
	%D%/div.c \
	%D%/dtoa.c \
	%D%/dtoastub.c \
	%D%/environ.c \
	%D%/envlock.c \
	%D%/eprintf.c \
	%D%/exit.c \
	%D%/freer.c \
	%D%/gdtoa-gethex.c \
	%D%/gdtoa-hexnan.c \
	%D%/getenv.c \
	%D%/getenv_r.c \
	%D%/imaxabs.c \
	%D%/imaxdiv.c \
	%D%/itoa.c \
	%D%/labs.c \
	%D%/ldiv.c \
	%D%/ldtoa.c \
	%D%/gdtoa-ldtoa.c \
	%D%/gdtoa-gdtoa.c \
	%D%/gdtoa-dmisc.c \
	%D%/gdtoa-gmisc.c \
	%D%/mallinfor.c \
	%D%/malloc.c \
	%D%/mallocr.c \
	%D%/mallstatsr.c \
	%D%/mblen.c \
	%D%/mblen_r.c \
	%D%/mbstowcs.c \
	%D%/mbstowcs_r.c \
	%D%/mbtowc.c \
	%D%/mbtowc_r.c \
	%D%/mlock.c \
	%D%/mprec.c \
	%D%/msizer.c \
	%D%/mstats.c \
	%D%/on_exit_args.c \
	%D%/quick_exit.c \
	%D%/rand.c \
	%D%/rand_r.c \
	%D%/random.c \
	%D%/realloc.c \
	%D%/reallocarray.c \
	%D%/reallocf.c \
	%D%/reallocr.c \
	%D%/sb_charsets.c \
	%D%/strtod.c \
	%D%/strtoimax.c \
	%D%/strtol.c \
	%D%/strtoul.c \
	%D%/strtoumax.c \
	%D%/utoa.c \
	%D%/wcstod.c \
	%D%/wcstoimax.c \
	%D%/wcstol.c \
	%D%/wcstoul.c \
	%D%/wcstoumax.c \
	%D%/wcstombs.c \
	%D%/wcstombs_r.c \
	%D%/wctomb.c \
	%D%/wctomb_r.c

if HAVE_LONG_DOUBLE
libc_a_SOURCES += \
	%D%/strtodg.c \
	%D%/strtold.c \
	%D%/strtorx.c \
	%D%/wcstold.c
endif # HAVE_LONG_DOUBLE

if NEWLIB_NANO_MALLOC
MALIGNR = nano-malignr
MALLOPTR = nano-malloptr
PVALLOCR = nano-pvallocr
VALLOCR = nano-vallocr
FREER = nano-freer
REALLOCR = nano-reallocr
CALLOCR = nano-callocr
CFREER = nano-cfreer
MALLINFOR = nano-mallinfor
MALLSTATSR = nano-mallstatsr
MSIZER = nano-msizer
MALLOCR = nano-mallocr
else
MALIGNR = malignr
MALLOPTR = malloptr
PVALLOCR = pvallocr
VALLOCR = vallocr
FREER = freer
REALLOCR = reallocr
CALLOCR = callocr
CFREER = cfreer
MALLINFOR = mallinfor
MALLSTATSR = mallstatsr
MSIZER = msizer
MALLOCR = mallocr
endif

libc_a_SOURCES += \
	%D%/arc4random.c \
	%D%/arc4random_uniform.c \
	%D%/cxa_atexit.c \
	%D%/cxa_finalize.c \
	%D%/drand48.c \
	%D%/ecvtbuf.c \
	%D%/efgcvt.c \
	%D%/erand48.c \
	%D%/jrand48.c \
	%D%/lcong48.c \
	%D%/lrand48.c \
	%D%/mrand48.c \
	%D%/msize.c \
	%D%/mtrim.c \
	%D%/nrand48.c \
	%D%/rand48.c \
	%D%/seed48.c \
	%D%/srand48.c \
	%D%/strtoll.c \
	%D%/strtoll_r.c \
	%D%/strtoull.c \
	%D%/strtoull_r.c \
	%D%/wcstoll.c \
	%D%/wcstoll_r.c \
	%D%/wcstoull.c \
	%D%/wcstoull_r.c \
	%D%/atoll.c \
	%D%/llabs.c \
	%D%/lldiv.c

%C%_ELIX_2_SOURCES = \
	%D%/a64l.c \
	%D%/btowc.c \
	%D%/getopt.c \
	%D%/getsubopt.c \
	%D%/l64a.c \
	%D%/malign.c \
	%D%/malignr.c \
	%D%/malloptr.c \
	%D%/mbrlen.c \
	%D%/mbrtowc.c \
	%D%/mbsinit.c \
	%D%/mbsnrtowcs.c \
	%D%/mbsrtowcs.c \
	%D%/on_exit.c \
	%D%/pvallocr.c \
	%D%/valloc.c \
	%D%/vallocr.c \
	%D%/wcrtomb.c \
	%D%/wcsnrtombs.c \
	%D%/wcsrtombs.c \
	%D%/wctob.c

%C%_ELIX_3_SOURCES = \
	%D%/putenv.c \
	%D%/putenv_r.c \
	%D%/setenv.c \
	%D%/setenv_r.c

%C%_ELIX_4_SOURCES = \
	%D%/rpmatch.c \
	%D%/system.c

if ELIX_LEVEL_1
%C%_ELIX_SOURCES =
else
if ELIX_LEVEL_2
%C%_ELIX_SOURCES = $(%C%_ELIX_2_SOURCES)
else
if ELIX_LEVEL_3
%C%_ELIX_SOURCES = $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_3_SOURCES)
else
%C%_ELIX_SOURCES = $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_3_SOURCES) $(%C%_ELIX_4_SOURCES)
endif
endif
endif
libc_a_SOURCES += $(%C%_ELIX_SOURCES)

LIBC_CHEWOUT_FILES += \
	%D%/_Exit.def \
	%D%/a64l.def \
	%D%/abort.def \
	%D%/abs.def \
	%D%/assert.def \
	%D%/atexit.def \
	%D%/atof.def \
	%D%/atoi.def \
	%D%/atoll.def \
	%D%/calloc.def \
	%D%/div.def \
	%D%/ecvtbuf.def \
	%D%/efgcvt.def \
	%D%/envlock.def \
	%D%/exit.def \
	%D%/getenv.def \
	%D%/itoa.def \
	%D%/labs.def \
	%D%/ldiv.def \
	%D%/llabs.def \
	%D%/lldiv.def \
	%D%/malloc.def \
	%D%/mblen.def \
	%D%/mbsnrtowcs.def \
	%D%/mbstowcs.def \
	%D%/mbtowc.def \
	%D%/mlock.def \
	%D%/mstats.def \
	%D%/on_exit.def \
	%D%/rand.def \
	%D%/rand48.def \
	%D%/random.def \
	%D%/rpmatch.def \
	%D%/strtod.def \
	%D%/strtol.def \
	%D%/strtoll.def \
	%D%/strtoul.def \
	%D%/strtoull.def \
	%D%/utoa.def \
	%D%/wcsnrtombs.def \
	%D%/wcstod.def \
	%D%/wcstol.def \
	%D%/wcstoll.def \
	%D%/wcstoul.def \
	%D%/wcstoull.def \
	%D%/system.def \
	%D%/wcstombs.def \
	%D%/wctomb.def

LIBC_CHAPTERS += %D%/stdlib.tex
