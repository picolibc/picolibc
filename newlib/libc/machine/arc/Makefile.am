## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES =			\
	memcmp.S		\
	memcmp-bs-norm.S	\
	memcmp-stub.c		\
	memcpy.S		\
	memcpy-archs.S		\
	memcpy-bs.S		\
	memcpy-stub.c		\
	memset.S		\
	memset-archs.S		\
	memset-bs.S		\
	memset-stub.c		\
	setjmp.S		\
	strchr.S		\
	strchr-bs.S		\
	strchr-bs-norm.S	\
	strchr-stub.c		\
	strcmp.S		\
	strcmp-archs.S		\
	strcmp-stub.c		\
	strcpy.S		\
	strcpy-bs.S		\
	strcpy-bs-arc600.S	\
	strcpy-stub.c		\
	strlen.S		\
	strlen-bs.S		\
	strlen-bs-norm.S	\
	strlen-stub.c		\
	strncpy.S		\
	strncpy-stub.c		\
	strncpy-bs.S

lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

pdf:
install-pdf:
