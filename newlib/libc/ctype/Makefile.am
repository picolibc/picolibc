## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	ctype_.c \
	isalnum.c \
	isalpha.c \
	isascii.c \
	iscntrl.c \
	isdigit.c \
	islower.c \
	isupper.c \
	isprint.c \
	ispunct.c \
	isspace.c \
	isxdigit.c \
	toascii.c \
	tolower.c \
	toupper.c \
	_tolower.c \
	_toupper.c

libctype_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libctype.la
libctype_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES= \
	isalnum.def	\
	isalpha.def	\
	isascii.def	\
	iscntrl.def	\
	isdigit.def	\
	islower.def	\
	isprint.def	\
	ispunct.def	\
	isspace.def	\
	isupper.def	\
	isxdigit.def	\
	toascii.def	\
	tolower.def	\
	toupper.def	

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/ctype.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref
