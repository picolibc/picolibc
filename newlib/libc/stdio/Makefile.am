## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

if NEWLIB_NANO_FORMATTED_IO
GENERAL_INT_FORMATTED_IO_SOURCES =
else
GENERAL_INT_FORMATTED_IO_SOURCES = \
	%D%/fiprintf.c			\
	%D%/fiscanf.c			\
	%D%/iprintf.c			\
	%D%/iscanf.c			\
	%D%/siprintf.c			\
	%D%/siscanf.c			\
	%D%/sniprintf.c			\
	%D%/vdiprintf.c			\
	%D%/vfprintf.c			\
	%D%/vfwprintf.c			\
	%D%/viprintf.c			\
	%D%/viscanf.c			\
	%D%/vsiprintf.c			\
	%D%/vsiscanf.c			\
	%D%/vsniprintf.c
endif

GENERAL_SOURCES = \
	%D%/$(GENERAL_INT_FORMATTED_IO_SOURCES) \
	%D%/clearerr.c			\
	%D%/fclose.c			\
	%D%/fdopen.c			\
	%D%/feof.c				\
	%D%/ferror.c			\
	%D%/fflush.c			\
	%D%/fgetc.c			\
	%D%/fgetpos.c			\
	%D%/fgets.c			\
	%D%/fileno.c			\
	%D%/findfp.c			\
	%D%/flags.c			\
	%D%/fopen.c			\
	%D%/fprintf.c			\
	%D%/fputc.c			\
	%D%/fputs.c			\
	%D%/fread.c			\
	%D%/freopen.c			\
	%D%/fscanf.c			\
	%D%/fseek.c			\
	%D%/fsetpos.c			\
	%D%/ftell.c			\
	%D%/fvwrite.c			\
	%D%/fwalk.c			\
	%D%/fwrite.c			\
	%D%/getc.c				\
	%D%/getchar.c			\
	%D%/getc_u.c			\
	%D%/getchar_u.c			\
	%D%/getdelim.c			\
	%D%/getline.c			\
	%D%/gets.c				\
	%D%/makebuf.c			\
	%D%/perror.c			\
	%D%/printf.c			\
	%D%/putc.c				\
	%D%/putchar.c			\
	%D%/putc_u.c			\
	%D%/putchar_u.c			\
	%D%/puts.c				\
	%D%/refill.c			\
	%D%/remove.c			\
	%D%/rename.c			\
	%D%/rewind.c			\
	%D%/rget.c				\
	%D%/scanf.c			\
	%D%/sccl.c				\
	%D%/setbuf.c			\
	%D%/setbuffer.c			\
	%D%/setlinebuf.c			\
	%D%/setvbuf.c			\
	%D%/snprintf.c			\
	%D%/sprintf.c			\
	%D%/sscanf.c			\
	%D%/stdio.c			\
	%D%/tmpfile.c			\
	%D%/tmpnam.c			\
	%D%/ungetc.c			\
	%D%/vdprintf.c			\
	%D%/vprintf.c			\
	%D%/vscanf.c			\
	%D%/vsnprintf.c			\
	%D%/vsprintf.c			\
	%D%/vsscanf.c			\
	%D%/wbuf.c				\
	%D%/wsetup.c

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_2_SOURCES =
else
if NEWLIB_NANO_FORMATTED_IO
ELIX_2_INT_FORMATTED_IO_SOURCES =
else
ELIX_2_INT_FORMATTED_IO_SOURCES = \
	%D%/asiprintf.c		\
	%D%/vasiprintf.c

endif !NEWLIB_NANO_FORMATTED_IO
ELIX_2_SOURCES = \
	%D%/$(ELIX_2_INT_FORMATTED_IO_SOURCES) \
	%D%/asprintf.c		\
	%D%/fcloseall.c		\
	%D%/fseeko.c		\
	%D%/ftello.c		\
	%D%/getw.c			\
	%D%/mktemp.c		\
	%D%/putw.c			\
	%D%/vasprintf.c
endif !ELIX_LEVEL_1

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_4_SOURCES =
else
if ELIX_LEVEL_2
ELIX_4_SOURCES =
else
if ELIX_LEVEL_3
ELIX_4_SOURCES =
else
if NEWLIB_NANO_FORMATTED_IO
ELIX_4_INT_FORMATTED_IO_SOURCES =
else
ELIX_4_INT_FORMATTED_IO_SOURCES = \
	%D%/asniprintf.c		\
	%D%/diprintf.c		\
	%D%/vasniprintf.c

endif !NEWLIB_NANO_FORMATTED_IO
ELIX_4_SOURCES = \
	%D%/$(ELIX_4_INT_FORMATTED_IO_SOURCES) \
	%D%/asnprintf.c		\
	%D%/clearerr_u.c		\
	%D%/dprintf.c		\
	%D%/feof_u.c		\
	%D%/ferror_u.c		\
	%D%/fflush_u.c		\
	%D%/fgetc_u.c		\
	%D%/fgets_u.c		\
	%D%/fgetwc.c		\
	%D%/fgetwc_u.c		\
	%D%/fgetws.c		\
	%D%/fgetws_u.c		\
	%D%/fileno_u.c		\
	%D%/fmemopen.c		\
	%D%/fopencookie.c		\
	%D%/fpurge.c		\
	%D%/fputc_u.c		\
	%D%/fputs_u.c		\
	%D%/fputwc.c		\
	%D%/fputwc_u.c		\
	%D%/fputws.c		\
	%D%/fputws_u.c		\
	%D%/fread_u.c		\
	%D%/fsetlocking.c		\
	%D%/funopen.c		\
	%D%/fwide.c			\
	%D%/fwprintf.c		\
	%D%/fwrite_u.c		\
	%D%/fwscanf.c		\
	%D%/getwc.c			\
	%D%/getwc_u.c		\
	%D%/getwchar.c		\
	%D%/getwchar_u.c		\
	%D%/open_memstream.c	\
	%D%/putwc.c			\
	%D%/putwc_u.c		\
	%D%/putwchar.c		\
	%D%/putwchar_u.c		\
	%D%/stdio_ext.c		\
	%D%/swprintf.c		\
	%D%/swscanf.c		\
	%D%/ungetwc.c		\
	%D%/vasnprintf.c		\
	%D%/vswprintf.c		\
	%D%/vswscanf.c		\
	%D%/vwprintf.c		\
	%D%/vwscanf.c		\
	%D%/wprintf.c		\
	%D%/wscanf.c

endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

if NEWLIB_NANO_FORMATTED_IO
GENERAL_SOURCES += \
	%D%/nano-vfprintf_float.c \
	%D%/nano-svfprintf.c \
	%D%/nano-svfscanf.c \
	%D%/nano-vfprintf.c \
	%D%/nano-vfprintf_i.c \
	%D%/nano-vfscanf.c \
	%D%/nano-vfscanf_i.c \
	%D%/nano-vfscanf_float.c
else
GENERAL_SOURCES += \
	%D%/svfiprintf.c \
	%D%/svfprintf.c \
	%D%/svfiscanf.c \
	%D%/svfscanf.c \
	%D%/vfiprintf.c \
	%D%/vfscanf.c \
	%D%/vfiscanf.c
endif
GENERAL_SOURCES += \
	%D%/svfiwprintf.c \
	%D%/svfwprintf.c \
	%D%/vfiwprintf.c \
	%D%/svfiwscanf.c \
	%D%/svfwscanf.c \
	%D%/vfiwscanf.c \
	%D%/vfwscanf.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(AM_V_CC)$(COMPILE)
