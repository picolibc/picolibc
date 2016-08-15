## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = 	\
	ctype_.c 	\
	isalnum.c 	\
	isalpha.c 	\
	iscntrl.c 	\
	isdigit.c 	\
	islower.c 	\
	isupper.c 	\
	isprint.c 	\
	ispunct.c 	\
	isspace.c 	\
	isxdigit.c 	\
	tolower.c 	\
	toupper.c

## The following handles EL/IX level 2 functions
if ELIX_LEVEL_1
ELIX_SOURCES =
else
ELIX_SOURCES = \
	isalnum_l.c	\
	isalpha_l.c	\
	isascii.c 	\
	isascii_l.c 	\
	isblank.c 	\
	isblank_l.c 	\
	iscntrl_l.c 	\
	isdigit_l.c 	\
	islower_l.c 	\
	isupper_l.c 	\
	isprint_l.c 	\
	ispunct_l.c 	\
	isspace_l.c 	\
	iswalnum.c 	\
	iswalnum_l.c 	\
	iswalpha.c 	\
	iswalpha_l.c 	\
	iswblank.c 	\
	iswblank_l.c 	\
	iswcntrl.c 	\
	iswcntrl_l.c 	\
	iswctype.c 	\
	iswctype_l.c 	\
	iswdigit.c 	\
	iswdigit_l.c 	\
	iswgraph.c 	\
	iswgraph_l.c 	\
	iswlower.c 	\
	iswlower_l.c 	\
	iswprint.c 	\
	iswprint_l.c 	\
	iswpunct.c 	\
	iswpunct_l.c 	\
	iswspace.c 	\
	iswspace_l.c 	\
	iswupper.c 	\
	iswupper_l.c 	\
	iswxdigit.c 	\
	iswxdigit_l.c 	\
	isxdigit_l.c 	\
	jp2uc.c 	\
	toascii.c 	\
	toascii_l.c 	\
	tolower_l.c 	\
	toupper_l.c	\
	towctrans.c 	\
	towctrans_l.c 	\
	towlower.c 	\
	towlower_l.c 	\
	towupper.c 	\
	towupper_l.c 	\
	wctrans.c 	\
	wctrans_l.c 	\
	wctype.c	\
	wctype_l.c
endif

libctype_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libctype.la
libctype_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES= \
	isalnum.def	\
	isalpha.def	\
	isascii.def	\
	isblank.def	\
	iscntrl.def	\
	isdigit.def	\
	islower.def	\
	isprint.def	\
	ispunct.def	\
	isspace.def	\
	isupper.def	\
	iswalnum.def	\
	iswalpha.def	\
	iswblank.def 	\
	iswcntrl.def 	\
	iswctype.def 	\
	iswdigit.def 	\
	iswgraph.def 	\
	iswlower.def 	\
	iswprint.def 	\
	iswpunct.def 	\
	iswspace.def 	\
	iswupper.def 	\
	iswxdigit.def 	\
	isxdigit.def	\
	toascii.def	\
	tolower.def	\
	toupper.def	\
	towctrans.def	\
	towlower.def	\
	towupper.def	\
	wctrans.def	\
	wctype.def

CHAPTERS = ctype.tex

$(lpfx)ctype_.$(oext): ctype_.c ctype_iso.h ctype_cp.h
