## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
  xdr_private.c \
  xdr.c \
  xdr_float.c \
  xdr_array.c \
  xdr_mem.c \
  xdr_rec.c \
  xdr_reference.c \
  xdr_sizeof.c

STDIO_SOURCES = \
  xdr_stdio.c

EXTRA_SRC = \
  xdr_float_vax.c

## None of these functions are specified by EL/IX
if ELIX_LEVEL_1
ELIX_SOURCES =
else
if ELIX_LEVEL_2
ELIX_SOURCES =
else
if ELIX_LEVEL_3
ELIX_SOURCES =
else
if ELIX_LEVEL_4
ELIX_SOURCES =
else
if HAVE_STDIO_DIR
ELIX_SOURCES = $(GENERAL_SOURCES) $(STDIO_SOURCES)
else
ELIX_SOURCES = $(GENERAL_SOURCES)
endif
endif
endif
endif
endif

EXTRA_DIST = README $(EXTRA_SRC)

libxdr_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libxdr.la
libxdr_la_SOURCES = dummy.c $(ELIX_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = dummy.c $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

SUFFIXES = .def

CHEWOUT_FILES =

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)

CLEANFILES = $(CHEWOUT_FILES) *.ref

include $(srcdir)/../../Makefile.shared
