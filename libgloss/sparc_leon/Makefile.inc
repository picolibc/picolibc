AM_CPPFLAGS_%C% = -I$(srcdir)/%D%

multilibtool_LIBRARIES += %D%/libleonbare.a
%C%_libleonbare_a_SOURCES = \
	close.c \
	fstat.c \
	getpid.c \
	isatty.c \
	kill.c \
	lseek.c \
	open.c \
	print.c \
	putnum.c \
	read.c \
	sbrk.c \
	stat.c \
	unlink.c \
	write.c \
	%D%/io.c \
	%D%/kernel.c \
	%D%/kernel_debug.c \
	%D%/kernel_debug_var.c \
	%D%/kernel_context.S \
	%D%/etrap.S \
	%D%/rtrap.S \
	%D%/etrap_fast.S \
	%D%/rtrap_fast.S \
	%D%/irqinstall.S \
	%D%/regwin.S \
	%D%/regwinflush.S \
	%D%/fpu.S \
	%D%/bdinit.S \
	%D%/contextswitch.c \
	%D%/busscan.S \
	%D%/irqtrap_fast.S \
	%D%/catch_interrupt.c \
	%D%/catch_interrupt_svt.c \
	%D%/catch_interrupt_mvt.c \
	%D%/catch_interrupt_pending.c \
	%D%/gettimeofday.c \
	%D%/times.c \
	%D%/rtc.c \
	%D%/lcpuinit.S \
	%D%/console_init.c \
	%D%/console.c \
	%D%/console_dbg.c \
	%D%/contextswitch.c \
	%D%/contextswitch_asm.S \
	%D%/_exit.c \
	%D%/amba.c \
	%D%/amba_dbg.c \
	%D%/amba_scan.c \
	%D%/amba_driver.c \
	%D%/timer.c \
	%D%/mutex.c \
	%D%/locore.S \
	%D%/locore_clean.S \
	%D%/locore_var.S \
	%D%/locore_var_svt.S \
	%D%/jiffies.c \
	%D%/mmu_asm.S \
	%D%/locore_svtdisp.S \
	%D%/locore_mvt_reset.S \
	%D%/locore_svt_reset.S \
	%D%/stop.S \
	%D%/initcalls.c \
	%D%/regwin_patch.c \
	%D%/cacheA.S \
	%D%/nocache.S

multilibtool_LIBRARIES += %D%/liblbthread.a
%C%_liblbthread_a_SOURCES = \
	%D%/kernel.c \
	%D%/kernel_debug.c \
	%D%/kernel_debug_var.c \
	%D%/kernel_context.S \
	%D%/kernel_mutex.c \
	%D%/kernel_thread.c \
	%D%/kernel_sched.c \
	%D%/kernel_queue.c \
	%D%/kernel_mm.c

multilibtool_LIBRARIES += %D%/libsmall.a
%C%_libsmall_a_SOURCES = \
	%D%/locore_atexit.c

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/pnpinit.o \
	%D%/pnpinit_simple.o \
	%D%/locore_svt.o \
	%D%/locore_mvt.o
libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/pnpinit.c \
	%D%/pnpinit_simple.c \
	%D%/locore_svt.S \
	%D%/locore_mvt.S

## Isn't actually installed anywhere?
## multilibtool_DATA += %D%/crti.o %D%/crtn.o
## libobjs_a_SOURCES += %D%/crti.S %D%/crtn.S

if !HAVE_MULTISUBDIR
includetoolasmleondir = $(includetooldir)/asm-leon
includetoolasmleon_DATA = $(wildcard $(srcdir)/%D%/asm-leon/*.h)
endif
