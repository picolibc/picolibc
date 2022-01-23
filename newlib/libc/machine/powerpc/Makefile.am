## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

AM_CFLAGS = -I $(srcdir)/../../stdio -I $(srcdir)/../../stdlib

lib_a_SOURCES = setjmp.S
lib_a_CFLAGS=$(AM_CFLAGS)
lib_a_LIBADD =
if HAVE_POWERPC_ALTIVEC
lib_a_LIBADD += \
	lib_a-vfprintf.o \
	lib_a-vfscanf.o \
	lib_a-vec_malloc.o \
	lib_a-vec_calloc.o \
	lib_a-vec_free.o \
	lib_a-vec_realloc.o \
	lib_a-vec_mallocr.o \
	lib_a-vec_callocr.o \
	lib_a-vec_reallocr.o
endif
if HAVE_POWERPC_SPE
lib_a_LIBADD += \
	lib_a-atosfix16.o \
	lib_a-atosfix32.o \
	lib_a-atosfix64.o \
	lib_a-atoufix16.o \
	lib_a-atoufix32.o \
	lib_a-atoufix64.o \
	lib_a-simdldtoa.o \
	lib_a-strtosfix16.o \
	lib_a-strtosfix32.o \
	lib_a-strtosfix64.o \
	lib_a-strtoufix16.o \
	lib_a-strtoufix32.o \
	lib_a-strtoufix64.o \
	lib_a-ufix64toa.o \
	lib_a-vfprintf.o \
	lib_a-vfscanf.o
endif
lib_a_DEPENDENCIES = $(lib_a_LIBADD)
EXTRA_lib_a_SOURCES = \
  vfprintf.c vfscanf.c \
  vec_malloc.c vec_calloc.c vec_free.c vec_realloc.c vec_mallocr.c \
  atosfix16.c atosfix32.c atosfix64.c atoufix16.c atoufix32.c atoufix64.c \
  simdldtoa.c strtosfix16.c strtosfix32.c strtosfix64.c strtoufix16.c \
  strtoufix32.c strtoufix64.c ufix64toa.c

lib_a-vec_reallocr.o: vec_mallocr.c
	$(CC) $(DEFS) -DINTERNAL_NEWLIB -DDEFINE_VECREALLOC $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(lib_a_CFLAGS) $(CFLAGS) -c -o lib_a-vec_reallocr.o `test -f 'vec_mallocr.c' || echo '$(srcdir)/'`vec_mallocr.c

lib_a-vec_callocr.o: vec_mallocr.c
	$(CC) $(DEFS) -DINTERNAL_NEWLIB -DDEFINE_VECCALLOC $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(lib_a_CFLAGS) $(CFLAGS) -c -o lib_a-vec_callocr.o `test -f 'vec_mallocr.c' || echo '$(srcdir)/'`vec_mallocr.c
