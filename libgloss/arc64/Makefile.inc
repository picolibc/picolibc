multilibtool_LIBRARIES += %D%/libnsim.a
%C%_libnsim_a_CPPFLAGS = -I$(srcdir)/arc
%C%_libnsim_a_SOURCES = \
	arc/libcfunc.c \
	arc/nsim-syscalls.c \
	arc/sbrk.c

multilibtool_LIBRARIES += %D%/libhl.a
%C%_libhl_a_CFLAGS = -mvolatile-di
%C%_libhl_a_CPPFLAGS = -I$(srcdir)/arc
%C%_libhl_a_SOURCES = \
	arc/arc-timer.c \
	arc/hl-stub.c \
	arc/hl-setup.c \
	arc/libcfunc.c \
	arc/sbrk.c \
	arc/hl/hl_gw.c \
	arc/hl/hl_api.c \
	arc/hl/hl_open.c \
	arc/hl/hl_close.c \
	arc/hl/hl_read.c \
	arc/hl/hl_write.c \
	arc/hl/hl_lseek.c \
	arc/hl/hl_unlink.c \
	arc/hl/hl_isatty.c \
	arc/hl/hl_clock.c \
	arc/hl/hl_gettimeofday.c \
	arc/hl/hl_argc.c \
	arc/hl/hl_argv.c \
	arc/hl/hl_exit.c

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/crtn.o \
	%D%/crti.o \
	arc/arc-main-helper.o \
	arc/nsim.specs \
	arc/hl.specs

libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/crtn.S \
	%D%/crti.S \
	arc/arc-main-helper.c
