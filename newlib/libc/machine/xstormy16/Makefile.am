## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

# hack: putting $(newlib_basedir)/libm/common into INCLUDES forces automake
# to output a definition for newlib_basedir.
INCLUDES = -I $(newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

lib_a_LIBADD = malloc.o free.o realloc.o calloc.o cfree.o malign.o \
	valloc.o pvalloc.o mallinfo.o mallstats.o msize.o mallopt.o \
	mallocr.o freer.o reallocr.o callocr.o

MALLOC_COMPILE = $(COMPILE)

malloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC -c $(srcdir)/tiny-malloc.c -o $@

free.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_FREE -c $(srcdir)/tiny-malloc.c -o $@

realloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_REALLOC -c $(srcdir)/tiny-malloc.c -o $@

calloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_CALLOC -c $(srcdir)/tiny-malloc.c -o $@

cfree.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_CFREE -c $(srcdir)/tiny-malloc.c -o $@

malign.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MEMALIGN -c $(srcdir)/tiny-malloc.c -o $@

valloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_VALLOC -c $(srcdir)/tiny-malloc.c -o $@

pvalloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_PVALLOC -c $(srcdir)/tiny-malloc.c -o $@

mallinfo.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLINFO -c $(srcdir)/tiny-malloc.c -o $@

mallstats.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_STATS -c $(srcdir)/tiny-malloc.c -o $@

msize.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_USABLE_SIZE -c $(srcdir)/tiny-malloc.c -o $@

mallopt.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOPT -c $(srcdir)/mallocr.c -o $@

mallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC -c $(srcdir)/mallocr.c -o $@

freer.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_FREE -c $(srcdir)/mallocr.c -o $@

reallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_REALLOC -c $(srcdir)/mallocr.c -o $@

callocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_CALLOC -c $(srcdir)/mallocr.c -o $@

