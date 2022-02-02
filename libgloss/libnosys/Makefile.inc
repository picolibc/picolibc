multilibtool_DATA += %D%/nosys.specs
multilibtool_LIBRARIES += %D%/libnosys.a
%C%_libnosys_a_SOURCES = \
	%D%/chown.c \
	%D%/close.c \
	%D%/environ.c \
	%D%/errno.c \
	%D%/execve.c \
	%D%/fork.c \
	%D%/fstat.c \
	%D%/getpid.c \
	%D%/gettod.c \
	%D%/isatty.c \
	%D%/kill.c \
	%D%/link.c \
	%D%/lseek.c \
	%D%/open.c \
	%D%/read.c \
	%D%/readlink.c \
	%D%/sbrk.c \
	%D%/stat.c \
	%D%/symlink.c \
	%D%/times.c \
	%D%/unlink.c \
	%D%/wait.c \
	%D%/write.c \
	%D%/_exit.c
