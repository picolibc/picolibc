## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = reg_memcpy.S reg_memset.S strcmp.S memcpy.S memset.S \
	setjmp.S h8sx_strcpy.S
lib_a_LIBADD = 
lib_a_DEPENDENCIES = 

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
