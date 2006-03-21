## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
aliases.c \
ces.c \
ces_iso2022.c \
converter.c \
ccs.c \
ces_euc.c \
ces_table.c \
iconv.c \
loaddata.c \
bialiasesi.c \
biccs.c \
bices.c

## The following interfaces are EL/IX level 2
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
aliases.$(oext) \
ces.$(oext) \
ces_iso2022.$(oext) \
converter.$(oext) \
ccs.$(oext) \
ces_euc.$(oext) \
ces_table.$(oext) \
iconv.$(oext) \
loaddata.$(oext) \
bialiasesi.$(oext) \
biccs.$(oext) \
bices.$(oext)
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


