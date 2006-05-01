## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

# hack: putting $(newlib_basedir)/libm/common into INCLUDES forces automake
# to output a definition for newlib_basedir.
INCLUDES = -I $(newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

lib_a_LIBADD = malloc.o free.o realloc.o calloc.o cfree.o malign.o \
	valloc.o pvalloc.o mallinfo.o mallstats.o msize.o mallopt.o \
	mallocr.o freer.o reallocr.o callocr.o

MALLOC_COMPILE = $(COMPILE)

$(lpfx)malloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)free.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_FREE -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)realloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_REALLOC -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)calloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_CALLOC -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)cfree.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_CFREE -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)malign.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MEMALIGN -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)valloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_VALLOC -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)pvalloc.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_PVALLOC -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)mallinfo.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLINFO -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)mallstats.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_STATS -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)msize.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_USABLE_SIZE -c $(srcdir)/tiny-malloc.c -o $@

$(lpfx)mallopt.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOPT -c $(srcdir)/mallocr.c -o $@

$(lpfx)mallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC -c $(srcdir)/mallocr.c -o $@

$(lpfx)freer.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_FREE -c $(srcdir)/mallocr.c -o $@

$(lpfx)reallocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_REALLOC -c $(srcdir)/mallocr.c -o $@

$(lpfx)callocr.o: mallocr.c
	$(MALLOC_COMPILE) -DDEFINE_CALLOC -c $(srcdir)/mallocr.c -o $@

