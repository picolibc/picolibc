## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

AM_CFLAGS = -I $(srcdir)/../../stdio -I $(srcdir)/../../stdlib

lib_a_SOURCES = setjmp.S
lib_a_CFLAGS=$(AM_CFLAGS)
if HAVE_POWERPC_ALTIVEC
lib_a_SOURCES += \
	vfprintf.c \
	vfscanf.c \
	vec_malloc.c \
	vec_calloc.c \
	vec_free.c \
	vec_realloc.c \
	vec_mallocr.c \
	vec_callocr.c \
	vec_reallocr.c
endif
if HAVE_POWERPC_SPE
lib_a_SOURCES += \
	atosfix16.c \
	atosfix32.c \
	atosfix64.c \
	atoufix16.c \
	atoufix32.c \
	atoufix64.c \
	simdldtoa.c \
	strtosfix16.c \
	strtosfix32.c \
	strtosfix64.c \
	strtoufix16.c \
	strtoufix32.c \
	strtoufix64.c \
	ufix64toa.c \
	vfprintf.c \
	vfscanf.c
endif
