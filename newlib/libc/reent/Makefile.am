## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	closer.c \
	reent.c \
	impure.c \
	execr.c \
	fstatr.c \
	linkr.c \
	lseekr.c \
	openr.c \
	readr.c \
	signalr.c \
	signgam.c \
	sbrkr.c \
	statr.c \
	timer.c \
	unlinkr.c \
	writer.c

libreent_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libreent.la
libreent_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = \
	closer.def \
	reent.def \
	execr.def \
	fstatr.def \
	linkr.def \
	lseekr.def \
	openr.def \
	readr.def \
	signalr.def \
	sbrkr.def \
	statr.def \
	timer.def \
	unlinkr.def \
	writer.def

SUFFIXES = .def .h

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/reent.tex >> $(TARGETDOC)

impure.$(oext): $(srcdir)/impure.c $(srcdir)/../include/sys/reent.h

CLEANFILES = $(CHEWOUT_FILES) *.ref
