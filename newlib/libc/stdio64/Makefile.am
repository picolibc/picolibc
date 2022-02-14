## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

# need a dummy object so objectlist.awk.in will build every time
GENERAL_SOURCES = dummy.c local64.h

## All interfaces are EL/IX level 2
if ELIX_LEVEL_1
ELIX_SOURCES =
else
ELIX_SOURCES = \
	fdopen64.c 	\
	fgetpos64.c	\
	fopen64.c 	\
	freopen64.c 	\
	fseeko64.c 	\
	fsetpos64.c 	\
	ftello64.c	\
	stdio64.c	\
	tmpfile64.c
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
LIB_COMPILE = $(AM_V_CC)$(COMPILE)

AM_CFLAGS = -I $(srcdir)/../stdio
