## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	__adjust.c 	\
	__exp10.c 	\
	__ten_mu.c 	\
	abort.c  	\
	abs.c 		\
	assert.c  	\
	atexit.c	\
	atof.c 		\
	atoff.c		\
	atoi.c  	\
	atol.c		\
	bsearch.c	\
	calloc.c	\
	div.c  		\
	dtoa.c 		\
	dtoastub.c 	\
	ecvtbuf.c 	\
	efgcvt.c 	\
	environ.c	\
	envlock.c	\
	eprintf.c	\
	exit.c 		\
	getenv.c  	\
	getenv_r.c	\
	getopt.c	\
	labs.c 		\
	ldiv.c  	\
	ldtoa.c		\
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
	msize.c		\
	mstats.c	\
	mtrim.c		\
	putenv.c	\
	putenv_r.c	\
	qsort.c		\
	rand.c		\
	rand_r.c	\
	realloc.c	\
	setenv.c	\
	setenv_r.c	\
	strdup.c	\
	strdup_r.c	\
	strtod.c	\
	strtol.c	\
	strtoul.c	\
	system.c	\
	valloc.c	\
	wcstombs.c	\
	wcstombs_r.c	\
	wctomb.c	\
	wctomb_r.c

lib_a_LIBADD = mallocr.o freer.o reallocr.o callocr.o cfreer.o malignr.o \
	vallocr.o pvallocr.o mallinfor.o mallstatsr.o msizer.o malloptr.o

MALLOC_COMPILE = $(COMPILE) -DINTERNAL_NEWLIB

mallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC -c $(srcdir)/mallocr.c -o $@

freer.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_FREE -c $(srcdir)/mallocr.c -o $@

reallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_REALLOC -c $(srcdir)/mallocr.c -o $@

callocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_CALLOC -c $(srcdir)/mallocr.c -o $@

cfreer.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_CFREE -c $(srcdir)/mallocr.c -o $@

malignr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MEMALIGN -c $(srcdir)/mallocr.c -o $@

vallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_VALLOC -c $(srcdir)/mallocr.c -o $@

pvallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_PVALLOC -c $(srcdir)/mallocr.c -o $@

mallinfor.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLINFO -c $(srcdir)/mallocr.c -o $@

mallstatsr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_STATS -c $(srcdir)/mallocr.c -o $@

msizer.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_USABLE_SIZE -c $(srcdir)/mallocr.c -o $@

malloptr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOPT -c $(srcdir)/mallocr.c -o $@

CHEWOUT_FILES= \
	abort.def	\
	abs.def		\
	assert.def	\
	atexit.def	\
	atof.def 	\
	ecvtbuf.def	\
	atoi.def 	\
	bsearch.def	\
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
	qsort.def	\
	rand.def	\
	strtod.def 	\
	strtol.def 	\
	strtoul.def 	\
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

dtoa.o: dtoa.c mprec.h
ldtoa.o: ldtoa.c mprec.h
ecvtbuf.o: ecvtbuf.c mprec.h
mbtowc_r.o: mbtowc_r.c mbctype.h
	$(COMPILE) -c -fshort-enums $(srcdir)/mbtowc_r.c -o $@

mprec.o: mprec.c mprec.h
strtod.o: strtod.c mprec.h
wctomb_r.o: wctomb_r.c mbctype.h
