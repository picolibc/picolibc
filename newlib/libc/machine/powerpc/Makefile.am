## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S times.c
lib_a_LIBADD = @extra_objs@
EXTRA_lib_a_SOURCES = @extra_sources@
lib_a_DEPENDENCIES = @extra_objs@

ACLOCAL_AMFLAGS = -I ../../.. 
AM_CFLAGS = -I $(srcdir)/../../stdio -I $(srcdir)/../../stdlib
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

VEC_MALLOC_COMPILE = $(COMPILE) -DINTERNAL_NEWLIB

vec_reallocr.o: vec_mallocr.c
	$(VEC_MALLOC_COMPILE) -DDEFINE_VECREALLOC -c $(srcdir)/vec_mallocr.c -o $@

vec_callocr.o: vec_mallocr.c
	$(VEC_MALLOC_COMPILE) -DDEFINE_VECCALLOC -c $(srcdir)/vec_mallocr.c -o $@

