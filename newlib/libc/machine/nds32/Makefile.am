## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

if IS_NDS32_ISA_V3M
lib_a_SOURCES = abort.c setjmp.S strcmp.S strcpy.S
else
lib_a_SOURCES = abort.c memcpy.S memset.S setjmp.S strcmp.S strcpy.S
endif
lib_a_CFLAGS = $(AM_CFLAGS)
