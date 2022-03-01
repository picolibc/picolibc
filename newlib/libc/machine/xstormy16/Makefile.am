## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S \
	calloc.c \
	callocr.c \
	cfree.c \
	freer.c \
	malign.c \
	malloc.c \
	mallocr.c \
	msize.c \
	mstats.c \
	realloc.c \
	reallocr.c \
	valloc.c
lib_a_CFLAGS = $(AM_CFLAGS)
