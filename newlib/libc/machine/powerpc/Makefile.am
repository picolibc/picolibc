## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

AM_CFLAGS = -I $(srcdir)/../../stdio -I $(srcdir)/../../stdlib

lib_a_SOURCES = setjmp.S
lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS=$(AM_CFLAGS)
lib_a_LIBADD = @extra_objs@
lib_a_DEPENDENCIES = @extra_objs@
EXTRA_lib_a_SOURCES = \
  vfprintf.c vfscanf.c \
  vec_malloc.c vec_calloc.c vec_free.c vec_realloc.c vec_mallocr.c \
  atosfix16.c atosfix32.c atosfix64.c atoufix16.c atoufix32.c atoufix64.c \
  simdldtoa.c strtosfix16.c strtosfix32.c strtosfix64.c strtoufix16.c \
  strtoufix32.c strtoufix64.c ufix64toa.c

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../.. 
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

lib_a-vec_reallocr.o: vec_mallocr.c
	$(CC) $(DEFS) -DINTERNAL_NEWLIB -DDEFINE_VECREALLOC $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(lib_a_CFLAGS) $(CFLAGS) -c -o lib_a-vec_reallocr.o `test -f 'vec_mallocr.c' || echo '$(srcdir)/'`vec_mallocr.c

lib_a-vec_callocr.o: vec_mallocr.c
	$(CC) $(DEFS) -DINTERNAL_NEWLIB -DDEFINE_VECCALLOC $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(lib_a_CFLAGS) $(CFLAGS) -c -o lib_a-vec_callocr.o `test -f 'vec_mallocr.c' || echo '$(srcdir)/'`vec_mallocr.c

