## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = euc-jp.c \
	euc-kr.c \
	euc-tw.c \
	gb2312.c \
	iso-10646-ucs-2.c \
	iso-10646-ucs-4.c \
	ucs-2-internal.c \
	ucs-4-internal.c \
	utf-16.c \
	utf-8.c

## The following interfaces are EL/IX level 2
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
	euc-jp.$(oext) \
	euc-kr.$(oext) \
	euc-tw.$(oext) \
	gb2312.$(oext) \
	iso-10646-ucs-2.$(oext) \
	iso-10646-ucs-4.$(oext) \
	ucs-2-internal.$(oext) \
	ucs-4-internal.$(oext) \
	utf-16.$(oext) \
	utf-8.$(oext)
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

