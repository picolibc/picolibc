AM_CPPFLAGS_%C% = -D_LIBGLOSS -I$(srcdir)/%D%/include

multilibtool_DATA += \
	%D%/default.specs \
	%D%/nano.specs \
	%D%/sim.elf.specs \
	%D%/board.elf.specs \
	%D%/sys.qemu.specs \
	%D%/sys.openocd.specs \
	%D%/crt0.o
libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/crt1-sim.S \
	%D%/crt1-boards.S

multilibtool_LIBRARIES += %D%/libgloss.a

if HAVE_XTENSA_BOARD_ESP
multilibtool_DATA += \
	%D%/app.elf.ld \
	%D%/crt1-sim.o \
	%D%/crt1-boards.o

%C%_libgloss_a_SOURCES = \
	%D%/clibrary_init.c \
	%D%/sleep.S \
	%D%/syscalls.c \
	%D%/window-vectors.S
%C%_libgloss_a_CPPFLAGS = $(AM_CPPFLAGS)

multilibtool_LIBRARIES += %D%/libsys_qemu.a
%C%_libsys_qemu_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DQEMU_SEMIHOSTING
%C%_libsys_qemu_a_SOURCES = \
	%D%/sim-vectors.S \
	%D%/sim-call.S \
	%D%/syscalls.c

multilibtool_LIBRARIES += %D%/libsys_openocd.a
%C%_libsys_openocd_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DOPENOCD_SEMIHOSTING
%C%_libsys_openocd_a_SOURCES = \
	%D%/syscalls.c

if HAVE_XTENSA_BOARD_ESP32
multilibtool_DATA += %D%/boards/esp32/memory.elf.ld
%C%_libgloss_a_SOURCES += %D%/boards/esp32/board.c
%C%_libgloss_a_CPPFLAGS += -I$(srcdir)/%D%/boards/esp32/include
%C%_libsys_qemu_a_CPPFLAGS += -I$(srcdir)/%D%/boards/esp32/include
%C%_libsys_openocd_a_CPPFLAGS += -I$(srcdir)/%D%/boards/esp32/include
endif

if HAVE_XTENSA_BOARD_ESP32S3
multilibtool_DATA += %D%/boards/esp32s3/memory.elf.ld
%C%_libgloss_a_SOURCES += %D%/boards/esp32s3/board.c
%C%_libgloss_a_CPPFLAGS += -I$(srcdir)/%D%/boards/esp32s3/include
%C%_libsys_qemu_a_CPPFLAGS += -I$(srcdir)/%D%/boards/esp32s3/include
%C%_libsys_openocd_a_CPPFLAGS += -I$(srcdir)/%D%/boards/esp32s3/include
endif
endif
