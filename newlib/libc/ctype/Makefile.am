## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
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
