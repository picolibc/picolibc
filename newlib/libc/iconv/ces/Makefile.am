## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	utf-8.c \
	utf-16.c \
	ucs-2.c \
	us-ascii.c \
	ucs-4.c \
	ucs-2-internal.c \
	ucs-4-internal.c \
	cesbi.c \
	table.c \
	table-pcs.c \
	euc.c

## The following interfaces are EL/IX level 2
if ELIX_LEVEL_1
LIB_OBJS = \
	utf-8.$(oext) \
	utf-16.$(oext) \
	ucs-2.$(oext) \
	us-ascii.$(oext) \
	ucs-4.$(oext) \
	ucs-2-internal.$(oext) \
	ucs-4-internal.$(oext) \
	cesbi.$(oext) \
	table.$(oext) \
	table-pcs.$(oext) \
	euc.$(oext)
else
LIB_OBJS = 
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

CHEWOUT_FILES = ces.def

SUFFIXES = .def

CHEW = ../../../doc/makedoc -f $(srcdir)/../../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/ces.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

