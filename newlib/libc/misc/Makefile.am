## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = __dprintf.c unctrl.c ffs.c init.c fini.c

if NEWLIB_RETARGETABLE_LOCKING
LIB_SOURCES += \
	lock.c
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
