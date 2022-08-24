# Makefile.am for Cygwin's testsuite.
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

# This makefile requires GNU make.

noinst_LIBRARIES = libltp.a

libltp_a_CPPFLAGS=-I$(srcdir)/libltp/include

libltp_a_SOURCES = \
	libltp/lib/dataascii.c \
	libltp/lib/databin.c \
	libltp/lib/datapid.c \
	libltp/lib/forker.c \
	libltp/lib/get_high_address.c \
	libltp/lib/libtestsuite.c \
	libltp/lib/open_flags.c \
	libltp/lib/parse_opts.c \
	libltp/lib/pattern.c \
	libltp/lib/rmobj.c \
	libltp/lib/search_path.c \
	libltp/lib/str_to_bytes.c \
	libltp/lib/string_to_tokens.c \
	libltp/lib/tst_res.c \
	libltp/lib/tst_sig.c \
	libltp/lib/tst_tmpdir.c \
	libltp/lib/write_log.c

check_PROGRAMS = \
	winsup.api/checksignal \
	winsup.api/crlf \
	winsup.api/devdsp \
	winsup.api/devzero \
	winsup.api/iospeed \
	winsup.api/mmaptest01 \
	winsup.api/mmaptest02 \
	winsup.api/mmaptest03 \
	winsup.api/mmaptest04 \
	winsup.api/msgtest \
	winsup.api/nullgetcwd \
	winsup.api/resethand \
	winsup.api/semtest \
	winsup.api/shmtest \
	winsup.api/sigchld \
	winsup.api/signal-into-win32-api \
	winsup.api/systemcall \
	winsup.api/user_malloc \
	winsup.api/waitpid \
	winsup.api/ltp/access01 \
	winsup.api/ltp/access03 \
	winsup.api/ltp/access04 \
	winsup.api/ltp/access05 \
	winsup.api/ltp/alarm01 \
	winsup.api/ltp/alarm02 \
	winsup.api/ltp/alarm03 \
	winsup.api/ltp/alarm07 \
	winsup.api/ltp/asyncio02 \
	winsup.api/ltp/chdir02 \
	winsup.api/ltp/chdir04 \
	winsup.api/ltp/chmod01 \
	winsup.api/ltp/chmod02 \
	winsup.api/ltp/chown01 \
	winsup.api/ltp/close01 \
	winsup.api/ltp/close02 \
	winsup.api/ltp/close08 \
	winsup.api/ltp/creat01 \
	winsup.api/ltp/creat03 \
	winsup.api/ltp/creat09 \
	winsup.api/ltp/dup01 \
	winsup.api/ltp/dup02 \
	winsup.api/ltp/dup03 \
	winsup.api/ltp/dup04 \
	winsup.api/ltp/dup05 \
	winsup.api/ltp/execl01 \
	winsup.api/ltp/execle01 \
	winsup.api/ltp/execlp01 \
	winsup.api/ltp/execv01 \
	winsup.api/ltp/execve01 \
	winsup.api/ltp/execvp01 \
	winsup.api/ltp/exit01 \
	winsup.api/ltp/exit02 \
	winsup.api/ltp/fchdir01 \
	winsup.api/ltp/fchdir02 \
	winsup.api/ltp/fchmod01 \
	winsup.api/ltp/fchown01 \
	winsup.api/ltp/fcntl02 \
	winsup.api/ltp/fcntl03 \
	winsup.api/ltp/fcntl04 \
	winsup.api/ltp/fcntl05 \
	winsup.api/ltp/fcntl07 \
	winsup.api/ltp/fcntl07B \
	winsup.api/ltp/fcntl08 \
	winsup.api/ltp/fcntl09 \
	winsup.api/ltp/fcntl10 \
	winsup.api/ltp/fork01 \
	winsup.api/ltp/fork02 \
	winsup.api/ltp/fork03 \
	winsup.api/ltp/fork04 \
	winsup.api/ltp/fork06 \
	winsup.api/ltp/fork07 \
	winsup.api/ltp/fork09 \
	winsup.api/ltp/fork10 \
	winsup.api/ltp/fork11 \
	winsup.api/ltp/fpathconf01 \
	winsup.api/ltp/fstat01 \
	winsup.api/ltp/fstat02 \
	winsup.api/ltp/fstat03 \
	winsup.api/ltp/fstat04 \
	winsup.api/ltp/fsync01 \
	winsup.api/ltp/ftruncate01 \
	winsup.api/ltp/ftruncate02 \
	winsup.api/ltp/ftruncate03 \
	winsup.api/ltp/getegid01 \
	winsup.api/ltp/geteuid01 \
	winsup.api/ltp/getgid01 \
	winsup.api/ltp/getgid02 \
	winsup.api/ltp/getgid03 \
	winsup.api/ltp/getgroups01 \
	winsup.api/ltp/getgroups02 \
	winsup.api/ltp/gethostid01 \
	winsup.api/ltp/gethostname01 \
	winsup.api/ltp/getpgid01 \
	winsup.api/ltp/getpgid02 \
	winsup.api/ltp/getpgrp01 \
	winsup.api/ltp/getpid01 \
	winsup.api/ltp/getpid02 \
	winsup.api/ltp/getppid01 \
	winsup.api/ltp/getppid02 \
	winsup.api/ltp/getuid01 \
	winsup.api/ltp/getuid02 \
	winsup.api/ltp/getuid03 \
	winsup.api/ltp/kill01 \
	winsup.api/ltp/kill02 \
	winsup.api/ltp/kill03 \
	winsup.api/ltp/kill04 \
	winsup.api/ltp/kill09 \
	winsup.api/ltp/link02 \
	winsup.api/ltp/link03 \
	winsup.api/ltp/link04 \
	winsup.api/ltp/link05 \
	winsup.api/ltp/lseek01 \
	winsup.api/ltp/lseek02 \
	winsup.api/ltp/lseek03 \
	winsup.api/ltp/lseek04 \
	winsup.api/ltp/lseek05 \
	winsup.api/ltp/lseek06 \
	winsup.api/ltp/lseek07 \
	winsup.api/ltp/lseek08 \
	winsup.api/ltp/lseek09 \
	winsup.api/ltp/lseek10 \
	winsup.api/ltp/lstat02 \
	winsup.api/ltp/mkdir01 \
	winsup.api/ltp/mkdir08 \
	winsup.api/ltp/mknod01 \
	winsup.api/ltp/mmap001 \
	winsup.api/ltp/mmap02 \
	winsup.api/ltp/mmap03 \
	winsup.api/ltp/mmap04 \
	winsup.api/ltp/mmap05 \
	winsup.api/ltp/mmap06 \
	winsup.api/ltp/mmap07 \
	winsup.api/ltp/mmap08 \
	winsup.api/ltp/munmap01 \
	winsup.api/ltp/munmap02 \
	winsup.api/ltp/nice05 \
	winsup.api/ltp/open02 \
	winsup.api/ltp/open03 \
	winsup.api/ltp/pathconf01 \
	winsup.api/ltp/pause01 \
	winsup.api/ltp/pipe01 \
	winsup.api/ltp/pipe08 \
	winsup.api/ltp/pipe09 \
	winsup.api/ltp/pipe10 \
	winsup.api/ltp/pipe11 \
	winsup.api/ltp/poll01 \
	winsup.api/ltp/read01 \
	winsup.api/ltp/read04 \
	winsup.api/ltp/readdir01 \
	winsup.api/ltp/readlink01 \
	winsup.api/ltp/readlink02 \
	winsup.api/ltp/readlink03 \
	winsup.api/ltp/rename01 \
	winsup.api/ltp/rename02 \
	winsup.api/ltp/rename08 \
	winsup.api/ltp/rename10 \
	winsup.api/ltp/rmdir01 \
	winsup.api/ltp/rmdir04 \
	winsup.api/ltp/rmdir05 \
	winsup.api/ltp/sbrk01 \
	winsup.api/ltp/select01 \
	winsup.api/ltp/select02 \
	winsup.api/ltp/select03 \
	winsup.api/ltp/setgid01 \
	winsup.api/ltp/setgroups01 \
	winsup.api/ltp/setpgid01 \
	winsup.api/ltp/setregid01 \
	winsup.api/ltp/setreuid01 \
	winsup.api/ltp/setuid01 \
	winsup.api/ltp/setuid02 \
	winsup.api/ltp/signal03 \
	winsup.api/ltp/stat01 \
	winsup.api/ltp/stat02 \
	winsup.api/ltp/stat03 \
	winsup.api/ltp/stat05 \
	winsup.api/ltp/stat06 \
	winsup.api/ltp/symlink01 \
	winsup.api/ltp/symlink02 \
	winsup.api/ltp/symlink03 \
	winsup.api/ltp/symlink04 \
	winsup.api/ltp/symlink05 \
	winsup.api/ltp/sync01 \
	winsup.api/ltp/sync02 \
	winsup.api/ltp/time01 \
	winsup.api/ltp/time02 \
	winsup.api/ltp/times01 \
	winsup.api/ltp/times02 \
	winsup.api/ltp/times03 \
	winsup.api/ltp/truncate01 \
	winsup.api/ltp/truncate02 \
	winsup.api/ltp/umask01 \
	winsup.api/ltp/umask02 \
	winsup.api/ltp/umask03 \
	winsup.api/ltp/uname01 \
	winsup.api/ltp/unlink05 \
	winsup.api/ltp/unlink06 \
	winsup.api/ltp/unlink07 \
	winsup.api/ltp/unlink08 \
	winsup.api/ltp/vfork01 \
	winsup.api/ltp/wait02 \
	winsup.api/ltp/wait401 \
	winsup.api/ltp/wait402 \
	winsup.api/ltp/write01 \
	winsup.api/ltp/write02 \
	winsup.api/ltp/write03 \
	winsup.api/pthread/cancel1 \
	winsup.api/pthread/cancel10 \
	winsup.api/pthread/cancel11 \
	winsup.api/pthread/cancel12 \
	winsup.api/pthread/cancel2 \
	winsup.api/pthread/cancel3 \
	winsup.api/pthread/cancel4 \
	winsup.api/pthread/cancel5 \
	winsup.api/pthread/cancel6 \
	winsup.api/pthread/cancel7 \
	winsup.api/pthread/cancel8 \
	winsup.api/pthread/cancel9 \
	winsup.api/pthread/cleanup2 \
	winsup.api/pthread/cleanup3 \
	winsup.api/pthread/condvar1 \
	winsup.api/pthread/condvar2 \
	winsup.api/pthread/condvar2_1 \
	winsup.api/pthread/condvar3 \
	winsup.api/pthread/condvar3_1 \
	winsup.api/pthread/condvar3_2 \
	winsup.api/pthread/condvar3_3 \
	winsup.api/pthread/condvar4 \
	winsup.api/pthread/condvar5 \
	winsup.api/pthread/condvar6 \
	winsup.api/pthread/condvar7 \
	winsup.api/pthread/condvar8 \
	winsup.api/pthread/condvar9 \
	winsup.api/pthread/count1 \
	winsup.api/pthread/create1 \
	winsup.api/pthread/create2 \
	winsup.api/pthread/equal1 \
	winsup.api/pthread/exit1 \
	winsup.api/pthread/exit2 \
	winsup.api/pthread/exit3 \
	winsup.api/pthread/inherit1 \
	winsup.api/pthread/join0 \
	winsup.api/pthread/join1 \
	winsup.api/pthread/join2 \
	winsup.api/pthread/mainthreadexits \
	winsup.api/pthread/mutex1 \
	winsup.api/pthread/mutex1d \
	winsup.api/pthread/mutex1e \
	winsup.api/pthread/mutex1n \
	winsup.api/pthread/mutex1r \
	winsup.api/pthread/mutex2 \
	winsup.api/pthread/mutex3 \
	winsup.api/pthread/mutex4 \
	winsup.api/pthread/mutex5 \
	winsup.api/pthread/mutex6d \
	winsup.api/pthread/mutex6e \
	winsup.api/pthread/mutex6n \
	winsup.api/pthread/mutex6r \
	winsup.api/pthread/mutex7 \
	winsup.api/pthread/mutex7d \
	winsup.api/pthread/mutex7e \
	winsup.api/pthread/mutex7n \
	winsup.api/pthread/mutex7r \
	winsup.api/pthread/mutex8e \
	winsup.api/pthread/mutex8n \
	winsup.api/pthread/mutex8r \
	winsup.api/pthread/once1 \
	winsup.api/pthread/priority1 \
	winsup.api/pthread/priority2 \
	winsup.api/pthread/rwlock1 \
	winsup.api/pthread/rwlock2 \
	winsup.api/pthread/rwlock3 \
	winsup.api/pthread/rwlock4 \
	winsup.api/pthread/rwlock5 \
	winsup.api/pthread/rwlock6 \
	winsup.api/pthread/rwlock7 \
	winsup.api/pthread/self1 \
	winsup.api/pthread/self2 \
	winsup.api/pthread/threadidafterfork \
	winsup.api/pthread/tsd1 \
	winsup.api/samples/sample-fail \
	winsup.api/samples/sample-pass
# winsup.api/ltp/ulimit01 is omitted as we don't have <ulimit.h>

# flags for linking against the just built implib
# TODO: use -nostdinc and to-be-installed headers as well?
LDFLAGS_FOR_TESTDLL = -nodefaultlibs
LDADD_FOR_TESTDLL = $(builddir)/../cygwin/libcygwin.a -lgcc -lkernel32 -luser32

# flags for test executables
AM_CPPFLAGS = -I$(srcdir)/libltp/include
AM_LDFLAGS = $(LDFLAGS_FOR_TESTDLL)
LDADD = $(builddir)/libltp.a $(builddir)/../cygwin/binmode.o $(LDADD_FOR_TESTDLL)

DEJATOOL = winsup

# Add '-v' to RUNTESTFLAGS if V=1
RUNTESTFLAGS_1 = -v
RUNTESTFLAGS = $(RUNTESTFLAGS_$(V))

# a temporary directory, to be used for files created by tests
tmpdir = $(abspath $(builddir)/tmp/)
# the same temporary directory, as an absolute, /cygdrive path (so it can be
# understood by the test DLL, which will have a different mount table)
testdll_tmpdir = $(shell cygpath -ma $(tmpdir) | sed -e 's#^\([A-Z]\):#/cygdrive/\L\1#')

site-extra.exp: ../config.status Makefile
	@rm -f ./tmp0
	@echo "set runtime_root \"`pwd`/runtime\"" >> ./tmp0
	@echo "set tmpdir $(tmpdir)" >> ./tmp0
	@echo "set testdll_tmpdir $(testdll_tmpdir)" >> ./tmp0
	@echo "set cygrun \"`pwd`/mingw/cygrun\"" >> ./tmp0
	@mv ./tmp0 site-extra.exp

EXTRA_DEJAGNU_SITE_CONFIG = site-extra.exp

# target to build all the programs needed by check, without running check
check_programs: $(check_PROGRAMS)

clean-local:
	rm -f *.log *.exe *.exp *.bak *.stackdump winsup.sum

if CROSS_BOOTSTRAP
SUBDIRS = mingw
endif
