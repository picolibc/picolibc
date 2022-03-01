## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S \
	callocr.c \
	freer.c \
	mallocr.c \
	mstats.c \
	reallocr.c
lib_a_CFLAGS = $(AM_CFLAGS)

lib_a_LIBADD = $(lpfx)malloc.o \
	$(lpfx)free.o \
	$(lpfx)realloc.o \
	$(lpfx)calloc.o \
	$(lpfx)cfree.o \
	$(lpfx)malign.o \
	$(lpfx)valloc.o \
	$(lpfx)pvalloc.o \
	$(lpfx)msize.o

lib_a_DEPENDENCIES = $(lib_a_LIBADD)

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

$(lpfx)msize.o: tiny-malloc.c
	$(MALLOC_COMPILE) -DDEFINE_MALLOC_USABLE_SIZE -c $(srcdir)/tiny-malloc.c -o $@
