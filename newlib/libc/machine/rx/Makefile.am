## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S \
 strncmp.S strcmp.S strncpy.S strcpy.S strlen.S strcat.S strncat.S \
 memset.S mempcpy.S memcpy.S memmove.S memchr.S
lib_a_CFLAGS=$(AM_CFLAGS)
