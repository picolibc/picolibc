## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

if HAVE_STDIO64_DIR
STDIO64_OBJECTS = \
	fstat64r.$(oext) \
	lseek64r.$(oext) \
	open64r.$(oext)

STDIO64_DEFS = \
	fstat64r.def \
	lseek64r.def \
	open64r.def
endif

ELIX_2_OBJS = $(STDIO64_OBJECTS)
ELIX_3_OBJS = execr.$(oext)

if ELIX_LEVEL_1
LIB_OBJS =
else
if ELIX_LEVEL_2
LIB_OBJS = $(ELIX_2_OBJS)
else
LIB_OBJS = $(ELIX_2_OBJS) $(ELIX_3_OBJS)
endif
endif

GENERAL_SOURCES = \
	closer.c \
	reent.c \
	impure.c \
	fcntlr.c \
	fstatr.c \
	getreent.c \
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
libreent_la_SOURCES = $(GENERAL_SOURCES)
libreent_la_LIBADD = $(LIB_OBJS)
libreent_la_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = \
	closer.def \
	reent.def \
	execr.def \
	fcntlr.def \
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
	$(STDIO64_DEFS) \
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
