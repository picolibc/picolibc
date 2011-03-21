## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

if MAY_SUPPLY_SYSCALLS
extra_objs = $(lpfx)_exit.o $(lpfx)access.o $(lpfx)chmod.o \
	$(lpfx)chown.o $(lpfx)close.o $(lpfx)creat.o $(lpfx)crt1.o \
	$(lpfx)execv.o $(lpfx)execve.o $(lpfx)fork.o $(lpfx)fstat.o \
	$(lpfx)getpid.o $(lpfx)isatty.o $(lpfx)kill.o $(lpfx)lseek.o \
	$(lpfx)open.o $(lpfx)pipe.o $(lpfx)read.o $(lpfx)link.o \
	$(lpfx)sbrk.o $(lpfx)stat.o $(lpfx)time.o $(lpfx)trap.o \
	$(lpfx)unlink.o $(lpfx)utime.o $(lpfx)wait.o $(lpfx)write.o \
	$(lpfx)times.o $(lpfx)gettime.o $(lpfx)rename.o
else
extra_objs =
endif

lib_a_LIBADD = $(extra_objs)
EXTRA_lib_a_SOURCES = \
	_exit.c access.c chmod.c chown.c close.c creat.c crt1.c \
	execv.c execve.c fork.c fstat.c getpid.c isatty.c \
	kill.c lseek.c open.c pipe.c read.c link.c \
	sbrk.c stat.c time.c trap.S unlink.c utime.c wait.c write.c \
	times.c gettime.c rename.c
lib_a_SOURCES =
lib_a_DEPENDENCIES = $(extra_objs)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

if MAY_SUPPLY_SYSCALLS
all-local: crt0.o
endif

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
