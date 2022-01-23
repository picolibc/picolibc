## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = divsi3.c mulsi3.c divhi3.S shifts.c cmpsi.c psi.S setjmp.S
lib_a_CFLAGS=$(AM_CFLAGS)
