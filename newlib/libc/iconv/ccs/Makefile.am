## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	big5.c \
	cp775.c \
	cp850.c \
	cp852.c \
	cp855.c \
	cp866.c \
	gb_2312_80.c \
	iso_8859_15.c \
	iso_8859_1.c \
	iso_8859_2.c \
	iso_8859_4.c \
	iso_8859_5.c \
	jis_x0201.c \
	jis_x0208_1983.c \
	jis_x0212_1990.c \
	koi8_r.c \
	koi8_u.c \
	ksx1001.c \
	shift_jis.c \
	cns11643_plane14.c \
	cns11643_plane1.c \
	cns11643_plane2.c \
	us_ascii.c

## The following interfaces are EL/IX level 2
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
	big5.$(oext) \
	cp775.$(oext) \
	cp850.$(oext) \
	cp852.$(oext) \
	cp855.$(oext) \
	cp866.$(oext) \
	gb_2312_80.$(oext) \
	iso_8859_15.$(oext) \
	iso_8859_1.$(oext) \
	iso_8859_2.$(oext) \
	iso_8859_4.$(oext) \
	iso_8859_5.$(oext) \
	jis_x0201.$(oext) \
	jis_x0208_1983.$(oext) \
	jis_x0212_1990.$(oext) \
	koi8_r.$(oext) \
	koi8_u.$(oext) \
	ksx1001.$(oext) \
	shift_jis.$(oext) \
	cns11643_plane14.$(oext) \
	cns11643_plane1.$(oext) \
	cns11643_plane2.$(oext) \
	us_ascii.$(oext)
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =

SUBDIRS=binary

CHEWOUT_FILES = ccs.def

SUFFIXES = .def

CHEW = ../../../doc/makedoc -f $(srcdir)/../../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/ccs.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

include $(srcdir)/../../../Makefile.shared
