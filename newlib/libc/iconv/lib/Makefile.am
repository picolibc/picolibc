## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
aliasesi.c \
ucsconv.c \
nullconv.c \
iconv.c \
aliasesbi.c \
iconvnls.c


## The following interfaces are EL/IX level 2
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
aliasesi.$(oext) \
ucsconv.$(oext) \
nullconv.$(oext) \
iconv.$(oext) \
aliasesbi.$(oext) \
iconvnls.$(oext)
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

CHEWOUT_FILES = iconv.def

SUFFIXES = .def

CHEW = ../../../doc/makedoc -f $(srcdir)/../../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

doc: $(CHEWOUT_FILES)

CLEANFILES = $(CHEWOUT_FILES) *.ref

