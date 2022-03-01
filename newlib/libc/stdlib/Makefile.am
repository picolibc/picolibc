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
	div.c  		\
	dtoa.c 		\
	dtoastub.c 	\
	environ.c	\
	envlock.c	\
	eprintf.c	\
	exit.c 		\
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
	malloc.c  	\
	mblen.c		\
	mblen_r.c	\
	mbstowcs.c	\
	mbstowcs_r.c	\
	mbtowc.c	\
	mbtowc_r.c	\
	mlock.c		\
	mprec.c		\
	mstats.c	\
	on_exit_args.c	\
	quick_exit.c	\
	rand.c		\
	rand_r.c	\
	random.c	\
	realloc.c	\
	reallocarray.c	\
	reallocf.c	\
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
	mbrlen.c	\
	mbrtowc.c	\
	mbsinit.c	\
	mbsnrtowcs.c	\
	mbsrtowcs.c	\
	on_exit.c	\
	valloc.c	\
	wcrtomb.c	\
	wcsnrtombs.c	\
	wcsrtombs.c	\
	wctob.c

ELIX_2_OBJS = \
	$(lpfx)malignr.o	\
	$(lpfx)malloptr.o \
	$(lpfx)pvallocr.o	\
	$(lpfx)vallocr.o

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
ELIX_OBJS =
else
if ELIX_LEVEL_2
ELIX_SOURCES = $(ELIX_2_SOURCES)
ELIX_OBJS = $(ELIX_2_OBJS)
else
if ELIX_LEVEL_3
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES)
ELIX_OBJS = $(ELIX_2_OBJS)
else
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES) $(ELIX_4_SOURCES)
ELIX_OBJS = $(ELIX_2_OBJS)
endif
endif
endif

# Because of how libtool moves objects around, mallocr must be built last.
LIBADD_OBJS = $(lpfx)freer.o $(lpfx)reallocr.o \
	$(lpfx)callocr.o $(lpfx)cfreer.o \
	$(lpfx)mallinfor.o $(lpfx)mallstatsr.o \
	$(lpfx)msizer.o $(lpfx)mallocr.o

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(EXTENDED_SOURCES) $(ELIX_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS) $(ELIX_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS) $(ELIX_OBJS)
LIB_COMPILE = $(AM_V_CC)$(COMPILE)

$(lpfx)mallocr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_MALLOC -c $< -o $@

$(lpfx)freer.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_FREE -c $< -o $@

$(lpfx)reallocr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_REALLOC -c $< -o $@

$(lpfx)callocr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_CALLOC -c $< -o $@

$(lpfx)cfreer.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_CFREE -c $< -o $@

$(lpfx)malignr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_MEMALIGN -c $< -o $@

$(lpfx)vallocr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_VALLOC -c $< -o $@

$(lpfx)pvallocr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_PVALLOC -c $< -o $@

$(lpfx)mallinfor.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_MALLINFO -c $< -o $@

$(lpfx)mallstatsr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_MALLOC_STATS -c $< -o $@

$(lpfx)msizer.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_MALLOC_USABLE_SIZE -c $< -o $@

$(lpfx)malloptr.o: mallocr.c
	$(LIB_COMPILE) -DDEFINE_MALLOPT -c $< -o $@

$(lpfx)dtoa.o: dtoa.c mprec.h
$(lpfx)ldtoa.o: ldtoa.c mprec.h gdtoa.h
$(lpfx)gdtoa-ldtoa.o: gdtoa-ldtoa.c mprec.h gdtoaimp.h gdtoa.h
$(lpfx)gdtoa-gdtoa.o: gdtoa-gdtoa.c mprec.h gdtoaimp.h gdtoa.h
$(lpfx)gdtoa-dmisc.o: gdtoa-dmisc.c mprec.h gdtoaimp.h gdtoa.h
$(lpfx)gdtoa-gmisc.o: gdtoa-gmisc.c mprec.h gdtoaimp.h gdtoa.h
$(lpfx)ecvtbuf.o: ecvtbuf.c mprec.h
$(lpfx)mbtowc_r.o: mbtowc_r.c mbctype.h
$(lpfx)mprec.o: mprec.c mprec.h
$(lpfx)strtod.o: strtod.c mprec.h
$(lpfx)gdtoa-gethex.o: gdtoa-gethex.c mprec.h
$(lpfx)gdtoa-hexnan.o: gdtoa-hexnan.c mprec.h
$(lpfx)wctomb_r.o: wctomb_r.c mbctype.h
$(lpfx)drand48.o: drand48.c rand48.h
$(lpfx)erand48.o: erand48.c rand48.h
$(lpfx)jrand48.o: jrand48.c rand48.h
$(lpfx)lcong48.o: lcong48.c rand48.h
$(lpfx)lrand48.o: lrand48.c rand48.h
$(lpfx)mrand48.o: mrand48.c rand48.h
$(lpfx)nrand48.o: nrand48.c rand48.h
$(lpfx)rand48.o: rand48.c rand48.h
$(lpfx)seed48.o: seed48.c rand48.h
$(lpfx)srand48.o: srand48.c rand48.h
