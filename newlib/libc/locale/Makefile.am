## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = setlocale.h locale.c localeconv.c

## The following interfaces are EL/IX level 2
if ELIX_LEVEL_1
ELIX_SOURCES =
else
ELIX_SOURCES = \
	duplocale.c \
	freelocale.c \
	lctype.c \
	lmessages.c \
	lnumeric.c \
	lmonetary.c \
	newlocale.c \
	nl_langinfo.c \
	timelocal.c \
	uselocale.c
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
