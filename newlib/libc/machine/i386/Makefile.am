## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I $(newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

if MACH_ADD_SETJMP
mach_add_src=setjmp.S
else
mach_add_src=
endif

lib_a_SOURCES = \
	memchr.S memcmp.S memcpy.S memset.S strchr.S \
	memmove.S strlen.S f_atan2.S f_atan2f.S f_exp.c f_expf.c \
	f_frexp.S f_frexpf.S f_log.S f_logf.S f_log10.S f_log10f.S \
	f_ldexp.S f_ldexpf.S f_pow.c f_powf.c f_tan.S f_tanf.S \
	$(mach_add_src)

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
