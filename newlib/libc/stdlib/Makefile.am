## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	__adjust.c 	\
	__exp10.c 	\
	__ten_mu.c 	\
	_Exit.c		\
	a64l.c		\
	abort.c  	\
	abs.c 		\
	assert.c  	\
	atexit.c	\
	atof.c 		\
	atoff.c		\
	atoi.c  	\
	atol.c		\
	calloc.c	\
	div.c  		\
	drand48.c	\
	dtoa.c 		\
	dtoastub.c 	\
	ecvtbuf.c 	\
	efgcvt.c 	\
	environ.c	\
	envlock.c	\
	eprintf.c	\
	erand48.c	\
	exit.c 		\
	getenv.c  	\
	getenv_r.c	\
	getopt.c	\
	getsubopt.c	\
	jrand48.c	\
	l64a.c		\
	labs.c 		\
	lcong48.c	\
	ldiv.c  	\
	ldtoa.c		\
	lrand48.c	\
	malign.c	\
	malloc.c  	\
	mblen.c		\
	mblen_r.c	\
	mbstowcs.c	\
	mbstowcs_r.c	\
	mbtowc.c	\
	mbtowc_r.c	\
	mlock.c		\
	mprec.c		\
	mrand48.c	\
	msize.c		\
	mstats.c	\
	mtrim.c		\
	nrand48.c	\
	on_exit.c	\
	putenv.c	\
	putenv_r.c	\
	rand.c		\
	rand48.c	\
	rand_r.c	\
	realloc.c	\
	seed48.c	\
	setenv.c	\
	setenv_r.c	\
	srand48.c	\
	strtod.c	\
	strtol.c	\
	strtoll.c	\
	strtoll_r.c	\
	strtoul.c	\
	strtoull.c	\
	strtoull_r.c	\
	system.c	\
	valloc.c	\
	wcstombs.c	\
	wcstombs_r.c	\
	wctomb.c	\
	wctomb_r.c	\
	btowc.c		\
	mbrlen.c	\
	mbrtowc.c	\
	mbsinit.c	\
	mbsrtowcs.c	\
	wcrtomb.c	\
	wcsrtombs.c	\
	wctob.c

# Because of how libtool moves objects around, mallocr must be built last.
LIBADD_OBJS = freer.$(oext) reallocr.$(oext) callocr.$(oext) cfreer.$(oext) malignr.$(oext) \
	vallocr.$(oext) pvallocr.$(oext) mallinfor.$(oext) mallstatsr.$(oext) msizer.$(oext) malloptr.$(oext) mallocr.$(oext)

libstdlib_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdlib.la
libstdlib_la_SOURCES = $(LIB_SOURCES)
libstdlib_la_LIBADD = $(LIBADD_OBJS)
LIB_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
LIB_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

MALLOC_COMPILE = $(LIB_COMPILE) -DINTERNAL_NEWLIB

mallocr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC -c $(srcdir)/mallocr.c -o $@

freer.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_FREE -c $(srcdir)/mallocr.c -o $@

reallocr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_REALLOC -c $(srcdir)/mallocr.c -o $@

callocr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_CALLOC -c $(srcdir)/mallocr.c -o $@

cfreer.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_CFREE -c $(srcdir)/mallocr.c -o $@

malignr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MEMALIGN -c $(srcdir)/mallocr.c -o $@

vallocr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_VALLOC -c $(srcdir)/mallocr.c -o $@

pvallocr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_PVALLOC -c $(srcdir)/mallocr.c -o $@

mallinfor.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLINFO -c $(srcdir)/mallocr.c -o $@

mallstatsr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_STATS -c $(srcdir)/mallocr.c -o $@

msizer.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_USABLE_SIZE -c $(srcdir)/mallocr.c -o $@

malloptr.$(oext): mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOPT -c $(srcdir)/mallocr.c -o $@

CHEWOUT_FILES= \
	_Exit.def	\
	a64l.def	\
	abort.def	\
	abs.def		\
	assert.def	\
	atexit.def	\
	atof.def 	\
	ecvtbuf.def	\
	atoi.def 	\
	calloc.def	\
	div.def		\
	efgcvt.def 	\
	envlock.def	\
	exit.def	\
	getenv.def	\
	labs.def	\
	ldiv.def	\
	malloc.def	\
	mallocr.def	\
	mblen.def	\
	mbstowcs.def	\
	mbtowc.def	\
	mlock.def	\
	mstats.def	\
	on_exit.def	\
	rand.def	\
	rand48.def	\
	strtod.def 	\
	strtol.def 	\
	strtoll.def 	\
	strtoul.def 	\
	strtoull.def 	\
	system.def	\
	wcstombs.def	\
	wctomb.def	

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/stdlib.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

dtoa.$(oext): dtoa.c mprec.h
ldtoa.$(oext): ldtoa.c mprec.h
ecvtbuf.$(oext): ecvtbuf.c mprec.h
mbtowc_r.$(oext): mbtowc_r.c mbctype.h
	$(LIB_COMPILE) -c -fshort-enums $(srcdir)/mbtowc_r.c -o $@

mprec.$(oext): mprec.c mprec.h
strtod.$(oext): strtod.c mprec.h
wctomb_r.$(oext): wctomb_r.c mbctype.h
drand48.$(oext): drand48.c rand48.h
erand48.$(oext): erand48.c rand48.h
jrand48.$(oext): jrand48.c rand48.h
lcong48.$(oext): lcong48.c rand48.h
lrand48.$(oext): lrand48.c rand48.h
mrand48.$(oext): mrand48.c rand48.h
nrand48.$(oext): nrand48.c rand48.h
rand48.$(oext): rand48.c rand48.h
seed48.$(oext): seed48.c rand48.h
srand48.$(oext): srand48.c rand48.h
