if HAVE_STDIO_DIR

if NEWLIB_NANO_FORMATTED_IO
libc_a_SOURCES += \
	%D%/nano-vfprintf_float.c \
	%D%/nano-svfprintf.c \
	%D%/nano-svfscanf.c \
	%D%/nano-vfprintf.c \
	%D%/nano-vfprintf_i.c \
	%D%/nano-vfscanf.c \
	%D%/nano-vfscanf_i.c \
	%D%/nano-vfscanf_float.c
else
libc_a_SOURCES += \
	%D%/fiprintf.c \
	%D%/fiscanf.c \
	%D%/iprintf.c \
	%D%/iscanf.c \
	%D%/siprintf.c \
	%D%/siscanf.c \
	%D%/sniprintf.c \
	%D%/svfiprintf.c \
	%D%/svfiscanf.c \
	%D%/svfprintf.c \
	%D%/svfscanf.c \
	%D%/vdiprintf.c \
	%D%/vfprintf.c \
	%D%/vfiprintf.c \
	%D%/vfiscanf.c \
	%D%/vfscanf.c \
	%D%/vfwprintf.c \
	%D%/viprintf.c \
	%D%/viscanf.c \
	%D%/vsiprintf.c \
	%D%/vsiscanf.c \
	%D%/vsniprintf.c
endif

libc_a_SOURCES += \
	%D%/clearerr.c \
	%D%/fclose.c \
	%D%/fdopen.c \
	%D%/feof.c \
	%D%/ferror.c \
	%D%/fflush.c \
	%D%/fgetc.c \
	%D%/fgetpos.c \
	%D%/fgets.c \
	%D%/fileno.c \
	%D%/findfp.c \
	%D%/flags.c \
	%D%/fopen.c \
	%D%/fprintf.c \
	%D%/fputc.c \
	%D%/fputs.c \
	%D%/fread.c \
	%D%/freopen.c \
	%D%/fscanf.c \
	%D%/fseek.c \
	%D%/fsetpos.c \
	%D%/ftell.c \
	%D%/fvwrite.c \
	%D%/fwalk.c \
	%D%/fwrite.c \
	%D%/getc.c \
	%D%/getchar.c \
	%D%/getc_u.c \
	%D%/getchar_u.c \
	%D%/getdelim.c \
	%D%/getline.c \
	%D%/gets.c \
	%D%/makebuf.c \
	%D%/perror.c \
	%D%/printf.c \
	%D%/putc.c \
	%D%/putchar.c \
	%D%/putc_u.c \
	%D%/putchar_u.c \
	%D%/puts.c \
	%D%/refill.c \
	%D%/remove.c \
	%D%/rename.c \
	%D%/rewind.c \
	%D%/rget.c \
	%D%/scanf.c \
	%D%/sccl.c \
	%D%/setbuf.c \
	%D%/setbuffer.c \
	%D%/setlinebuf.c \
	%D%/setvbuf.c \
	%D%/snprintf.c \
	%D%/sprintf.c \
	%D%/sscanf.c \
	%D%/stdio.c \
	%D%/svfiwprintf.c \
	%D%/svfiwscanf.c \
	%D%/svfwprintf.c \
	%D%/svfwscanf.c \
	%D%/tmpfile.c \
	%D%/tmpnam.c \
	%D%/ungetc.c \
	%D%/vdprintf.c \
	%D%/vfiwprintf.c \
	%D%/vfiwscanf.c \
	%D%/vfwscanf.c \
	%D%/vprintf.c \
	%D%/vscanf.c \
	%D%/vsnprintf.c \
	%D%/vsprintf.c \
	%D%/vsscanf.c \
	%D%/wbuf.c \
	%D%/wsetup.c

## The following are EL/IX level 2 interfaces
if !ELIX_LEVEL_1
%C%_ELIX_2_SOURCES = \
	%D%/asprintf.c \
	%D%/fcloseall.c \
	%D%/fseeko.c \
	%D%/ftello.c \
	%D%/getw.c \
	%D%/mktemp.c \
	%D%/putw.c \
	%D%/vasprintf.c
if !NEWLIB_NANO_FORMATTED_IO
%C%_ELIX_2_SOURCES += \
	%D%/asiprintf.c \
	%D%/vasiprintf.c
endif !NEWLIB_NANO_FORMATTED_IO
endif !ELIX_LEVEL_1

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
%C%_ELIX_4_SOURCES =
else
if ELIX_LEVEL_2
%C%_ELIX_4_SOURCES =
else
if ELIX_LEVEL_3
%C%_ELIX_4_SOURCES =
else
%C%_ELIX_4_SOURCES = \
	%D%/asnprintf.c \
	%D%/clearerr_u.c \
	%D%/dprintf.c \
	%D%/feof_u.c \
	%D%/ferror_u.c \
	%D%/fflush_u.c \
	%D%/fgetc_u.c \
	%D%/fgets_u.c \
	%D%/fgetwc.c \
	%D%/fgetwc_u.c \
	%D%/fgetws.c \
	%D%/fgetws_u.c \
	%D%/fileno_u.c \
	%D%/fmemopen.c \
	%D%/fopencookie.c \
	%D%/fpurge.c \
	%D%/fputc_u.c \
	%D%/fputs_u.c \
	%D%/fputwc.c \
	%D%/fputwc_u.c \
	%D%/fputws.c \
	%D%/fputws_u.c \
	%D%/fread_u.c \
	%D%/fsetlocking.c \
	%D%/funopen.c \
	%D%/fwide.c \
	%D%/fwprintf.c \
	%D%/fwrite_u.c \
	%D%/fwscanf.c \
	%D%/getwc.c \
	%D%/getwc_u.c \
	%D%/getwchar.c \
	%D%/getwchar_u.c \
	%D%/open_memstream.c \
	%D%/putwc.c \
	%D%/putwc_u.c \
	%D%/putwchar.c \
	%D%/putwchar_u.c \
	%D%/stdio_ext.c \
	%D%/swprintf.c \
	%D%/swscanf.c \
	%D%/ungetwc.c \
	%D%/vasnprintf.c \
	%D%/vswprintf.c \
	%D%/vswscanf.c \
	%D%/vwprintf.c \
	%D%/vwscanf.c \
	%D%/wprintf.c \
	%D%/wscanf.c
if !NEWLIB_NANO_FORMATTED_IO
%C%_ELIX_4_SOURCES += \
	%D%/asniprintf.c \
	%D%/diprintf.c \
	%D%/vasniprintf.c
endif !NEWLIB_NANO_FORMATTED_IO
endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

libc_a_SOURCES += $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_4_SOURCES)

endif

LIBC_CHEWOUT_FILES += \
	%D%/clearerr.def \
	%D%/diprintf.def \
	%D%/dprintf.def \
	%D%/fcloseall.def \
	%D%/fclose.def \
	%D%/fdopen.def \
	%D%/feof.def \
	%D%/ferror.def \
	%D%/fflush.def \
	%D%/fgetc.def \
	%D%/fgetpos.def \
	%D%/fgets.def \
	%D%/fgetwc.def \
	%D%/fgetws.def \
	%D%/fileno.def \
	%D%/fmemopen.def \
	%D%/fopen.def \
	%D%/fopencookie.def \
	%D%/fpurge.def \
	%D%/fputc.def \
	%D%/fputs.def \
	%D%/fputwc.def \
	%D%/fputws.def \
	%D%/fread.def \
	%D%/freopen.def \
	%D%/fseek.def \
	%D%/fsetlocking.def \
	%D%/fsetpos.def \
	%D%/ftell.def \
	%D%/funopen.def \
	%D%/fwide.def \
	%D%/fwrite.def \
	%D%/getc.def \
	%D%/getc_u.def \
	%D%/getchar.def \
	%D%/getchar_u.def \
	%D%/getdelim.def \
	%D%/getline.def \
	%D%/gets.def \
	%D%/getw.def \
	%D%/getwchar.def \
	%D%/mktemp.def \
	%D%/nano-vfprintf.def \
	%D%/nano-vfscanf.def \
	%D%/open_memstream.def \
	%D%/perror.def \
	%D%/putc.def \
	%D%/putc_u.def \
	%D%/putchar.def \
	%D%/putchar_u.def \
	%D%/puts.def \
	%D%/putw.def \
	%D%/putwchar.def \
	%D%/remove.def \
	%D%/rename.def \
	%D%/rewind.def \
	%D%/setbuf.def \
	%D%/setbuffer.def \
	%D%/setlinebuf.def \
	%D%/setvbuf.def \
	%D%/siprintf.def \
	%D%/siscanf.def \
	%D%/sprintf.def \
	%D%/sscanf.def \
	%D%/stdio_ext.def \
	%D%/swprintf.def \
	%D%/swscanf.def \
	%D%/tmpfile.def \
	%D%/tmpnam.def \
	%D%/ungetc.def \
	%D%/ungetwc.def \
	%D%/vfprintf.def \
	%D%/vfscanf.def \
	%D%/vfwprintf.def \
	%D%/vfwscanf.def \
	%D%/viprintf.def \
	%D%/viscanf.def

LIBC_CHAPTERS += %D%/stdio.tex
