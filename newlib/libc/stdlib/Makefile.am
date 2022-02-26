## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	__adjust.c 	\
	__atexit.c	\
	__call_atexit.c	\
	__exp10.c 	\
	__ten_mu.c 	\
	_Exit.c		\
	abort.c  	\
	abs.c 		\
	aligned_alloc.c	\
	assert.c  	\
	atexit.c	\
	atof.c 		\
	atoff.c		\
	atoi.c  	\
	atol.c		\
	calloc.c	\
	callocr.c	\
	cfreer.c	\
	div.c  		\
	dtoa.c 		\
	dtoastub.c 	\
	environ.c	\
	envlock.c	\
	eprintf.c	\
	exit.c 		\
	freer.c		\
	gdtoa-gethex.c	\
	gdtoa-hexnan.c	\
	getenv.c  	\
	getenv_r.c	\
	imaxabs.c	\
	imaxdiv.c	\
	itoa.c          \
	labs.c 		\
	ldiv.c  	\
	ldtoa.c		\
	gdtoa-ldtoa.c	\
	gdtoa-gdtoa.c	\
	gdtoa-dmisc.c	\
	gdtoa-gmisc.c	\
	mallinfor.c	\
	malloc.c  	\
	mallocr.c	\
	mallstatsr.c	\
	mblen.c		\
	mblen_r.c	\
	mbstowcs.c	\
	mbstowcs_r.c	\
	mbtowc.c	\
	mbtowc_r.c	\
	mlock.c		\
	mprec.c		\
	msizer.c	\
	mstats.c	\
	on_exit_args.c	\
	quick_exit.c	\
	rand.c		\
	rand_r.c	\
	random.c	\
	realloc.c	\
	reallocarray.c	\
	reallocf.c	\
	reallocr.c	\
	sb_charsets.c	\
	strtod.c	\
	strtoimax.c	\
	strtol.c	\
	strtoul.c	\
	strtoumax.c	\
	utoa.c          \
	wcstod.c	\
	wcstoimax.c	\
	wcstol.c	\
	wcstoul.c	\
	wcstoumax.c	\
	wcstombs.c	\
	wcstombs_r.c	\
	wctomb.c	\
	wctomb_r.c

if HAVE_LONG_DOUBLE
GENERAL_SOURCES += \
	strtodg.c	\
	strtold.c	\
	strtorx.c	\
	wcstold.c
endif # HAVE_LONG_DOUBLE

EXTENDED_SOURCES = \
	arc4random.c	\
	arc4random_uniform.c \
	cxa_atexit.c	\
	cxa_finalize.c	\
	drand48.c	\
	ecvtbuf.c	\
	efgcvt.c	\
	erand48.c	\
	jrand48.c	\
	lcong48.c	\
	lrand48.c	\
	mrand48.c	\
	msize.c		\
	mtrim.c		\
	nrand48.c	\
	rand48.c	\
	seed48.c	\
	srand48.c	\
	strtoll.c	\
	strtoll_r.c	\
	strtoull.c	\
	strtoull_r.c	\
	wcstoll.c	\
	wcstoll_r.c	\
	wcstoull.c	\
	wcstoull_r.c	\
	atoll.c		\
	llabs.c		\
	lldiv.c

ELIX_2_SOURCES = \
	a64l.c		\
	btowc.c		\
	getopt.c	\
	getsubopt.c	\
	l64a.c		\
	malign.c	\
	malignr.c	\
	malloptr.c	\
	mbrlen.c	\
	mbrtowc.c	\
	mbsinit.c	\
	mbsnrtowcs.c	\
	mbsrtowcs.c	\
	on_exit.c	\
	pvallocr.c	\
	valloc.c	\
	vallocr.c	\
	wcrtomb.c	\
	wcsnrtombs.c	\
	wcsrtombs.c	\
	wctob.c

ELIX_3_SOURCES = \
	putenv.c	\
	putenv_r.c	\
	setenv.c	\
	setenv_r.c

ELIX_4_SOURCES = \
	rpmatch.c	\
	system.c

if ELIX_LEVEL_1
ELIX_SOURCES =
else
if ELIX_LEVEL_2
ELIX_SOURCES = $(ELIX_2_SOURCES)
else
if ELIX_LEVEL_3
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES)
else
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES) $(ELIX_4_SOURCES)
endif
endif
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(EXTENDED_SOURCES) $(ELIX_SOURCES)
lib_a_LIBADD =
lib_a_CFLAGS = $(AM_CFLAGS)
