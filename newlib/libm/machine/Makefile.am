## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

SUBDIRS = $(libm_machine_dir) .

if USE_LIBTOOL
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES =
lib_a_LIBADD = $(machlib)
endif # USE_LIBTOOL

ACLOCAL_AMFLAGS = -I ../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
