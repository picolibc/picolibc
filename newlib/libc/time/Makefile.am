## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	asctime.c	\
	asctime_r.c	\
	clock.c		\
	ctime.c		\
	ctime_r.c	\
	difftime.c	\
	gmtime.c	\
	gmtime_r.c	\
	lcltime.c	\
	lcltime_r.c	\
	mktime.c	\
	strftime.c  	\
	time.c

CHEWOUT_FILES = \
	asctime.def	\
	clock.def 	\
	ctime.def	\
	difftime.def	\
	gmtime.def	\
	lcltime.def	\
	mktime.def	\
	strftime.def	\
	time.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/time.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref
