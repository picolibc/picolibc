## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I $(newlib_basedir)/../newlib/libm/common $(NEWLIB_CFLAGS) \
	$(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	s_ceil.c \
	s_floor.c \
	s_fma.c \
	s_fmax.c \
	s_fmin.c \
	s_llrint.c \
	s_llround.c \
	s_lrint.c \
	s_lround.c \
	s_nearbyint.c \
	s_rint.c \
	s_round.c \
	s_trunc.c \
	sf_ceil.c \
	sf_floor.c \
	sf_fma.c \
	sf_fmax.c \
	sf_fmin.c \
	sf_llrint.c \
	sf_llround.c \
	sf_lrint.c \
	sf_lround.c \
	sf_nearbyint.c \
	sf_rint.c \
	sf_round.c \
	sf_trunc.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
