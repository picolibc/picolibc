## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	ioctl.s isatty.c read.s lseek.s close.s sbrk.c fstat.s cerror.s \
	_exit.s write.s open.s signal.s kill.s getpid.s brk.s fork.s wait.s \
	execve.s exec.c utime.s fcntl.s chmod.s getuid.s getgid.s time.s \
	stat.s link.s unlink.s rename.s mkdir.s rmdir.s sigaction.s \
	sigprocmask.s waitpid.s sysconf.s pathconf.s fpathconf.s \
	getdents.s opendir.c readdir.c rewinddir.c closedir.c pipe.s \
	getgroups.s setuid.s getuid.s setgid.s geteuid.s getegid.s \
	chdir.s dup2.c dup.c tcgetattr.c tcsetattr.c speed.c tcline.c \
	times.s pause.s sleep.c alarm.s access.s _longjmp.s _setjmp.s

all: crt0.o

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
