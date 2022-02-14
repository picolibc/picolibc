## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = strlen.c strcmp.c strcpy.c setjmp.S longjmp.S
lib_a_CFLAGS=$(AM_CFLAGS)
