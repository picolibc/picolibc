multilibtool_LIBRARIES += %D%/libnsim.a
%C%_libnsim_a_SOURCES = \
	%D%/libcfunc.c \
	%D%/mcount.c \
	%D%/nsim-syscalls.c \
	%D%/sbrk.c

multilibtool_LIBRARIES += %D%/libhl.a
%C%_libhl_a_CPPFLAGS = -I$(srcdir)/%D%
%C%_libhl_a_SOURCES = \
	%D%/arc-timer.c \
	%D%/hl-stub.c \
	%D%/hl-setup.c \
	%D%/libcfunc.c \
	%D%/sbrk.c \
	%D%/mcount.c \
	%D%/hl/hl_gw.c \
	%D%/hl/hl_api.c \
	%D%/hl/hl_open.c \
	%D%/hl/hl_close.c \
	%D%/hl/hl_read.c \
	%D%/hl/hl_write.c \
	%D%/hl/hl_lseek.c \
	%D%/hl/hl_unlink.c \
	%D%/hl/hl_isatty.c \
	%D%/hl/hl_fstat.c \
	%D%/hl/hl_clock.c \
	%D%/hl/hl_gettimeofday.c \
	%D%/hl/hl_argc.c \
	%D%/hl/hl_argv.c \
	%D%/hl/hl_exit.c

multilibtool_LIBRARIES += %D%/libuart_8250.a
%C%_libuart_8250_a_CPPFLAGS = -I$(srcdir)/%D%
%C%_libuart_8250_a_SOURCES = \
	%D%/uart-8250.c \
	%D%/uart-8250-stub.c \
	%D%/sbrk.c \
	%D%/mcount.c

multilibtool_LIBRARIES += %D%/libiotdk_uart.a
%C%_libiotdk_uart_a_CPPFLAGS = -I$(srcdir)/%D%
%C%_libiotdk_uart_a_SOURCES = %D%/iotdk-uart-setup.c

multilibtool_LIBRARIES += %D%/libhsdk_uart.a
%C%_libhsdk_uart_a_CPPFLAGS = -I$(srcdir)/%D%
%C%_libhsdk_uart_a_SOURCES = %D%/hsdk-uart-setup.c

multilibtool_LIBRARIES += %D%/libemsk_uart.a
%C%_libemsk_uart_a_CPPFLAGS = -I$(srcdir)/%D%
%C%_libemsk_uart_a_SOURCES = %D%/emsk-uart-setup.c

multilibtool_LIBRARIES += %D%/libemsdp_uart.a
%C%_libemsdp_uart_a_CPPFLAGS = -I$(srcdir)/%D%
%C%_libemsdp_uart_a_SOURCES = %D%/emsdp-uart-setup.c

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/gcrt0.o \
	%D%/arc-main-helper.o \
	%D%/nano.specs \
	%D%/nsim.specs \
	%D%/hl.specs \
	%D%/arcv2elf-common.ld \
	%D%/emsdp1.1_ram.specs \
	%D%/emsdp1.1_ram.ld \
	%D%/emsdp1.1.specs \
	%D%/emsdp1.1.ld \
	%D%/emsdp1.2_ram.specs \
	%D%/emsdp1.2_ram.ld \
	%D%/emsdp1.2.specs \
	%D%/emsdp1.2.ld \
	%D%/emsk1_em4.specs \
	%D%/emsk1_em4.ld \
	%D%/emsk1_em6_ram.specs \
	%D%/emsk1_em6_ram.ld \
	%D%/emsk1_em6.specs \
	%D%/emsk1_em6.ld \
	%D%/emsk2.1_em5d.specs \
	%D%/emsk2.1_em5d.ld \
	%D%/emsk2.1_em7d_ram.specs \
	%D%/emsk2.1_em7d_ram.ld \
	%D%/emsk2.1_em7d.specs \
	%D%/emsk2.1_em7d.ld \
	%D%/emsk2.2_em7d_ram.specs \
	%D%/emsk2.2_em7d_ram.ld \
	%D%/emsk2.2_em7d.specs \
	%D%/emsk2.2_em7d.ld \
	%D%/emsk2.2_em9d_ram.specs \
	%D%/emsk2.2_em9d_ram.ld \
	%D%/emsk2.2_em9d.specs \
	%D%/emsk2.2_em9d.ld \
	%D%/emsk2.2_em11d_ram.specs \
	%D%/emsk2.2_em11d_ram.ld \
	%D%/emsk2.2_em11d.specs \
	%D%/emsk2.2_em11d.ld \
	%D%/hsdk.specs \
	%D%/hsdk.ld \
	%D%/iotdk.specs \
	%D%/iotdk.ld

libobjs_a_SOURCES += %D%/crt0.S %D%/gcrt0.S %D%/arc-main-helper.c
