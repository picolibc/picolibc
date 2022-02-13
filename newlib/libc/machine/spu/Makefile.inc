libc_a_SOURCES += \
	%D%/setjmp.S %D%/assert.c %D%/clearerr.c %D%/creat.c %D%/fclose.c %D%/feof.c \
	%D%/ferror.c %D%/fflush.c %D%/fgetc.c %D%/fgetpos.c %D%/fgets.c %D%/fileno.c %D%/fiprintf.S \
	%D%/fiscanf.S %D%/fopen.c %D%/fprintf.S %D%/fputc.c %D%/fputs.c %D%/fread.c %D%/freopen.c \
	%D%/fscanf.S %D%/fseek.c %D%/fsetpos.c %D%/ftell.c %D%/fwrite.c %D%/getc.c %D%/getchar.c \
	%D%/gets.c %D%/impure.c %D%/iprintf.S %D%/iscanf.S %D%/memcmp.c %D%/memcpy.c %D%/memmove.c \
	%D%/memset.c %D%/perror.c %D%/printf.S %D%/putc.c %D%/putchar.c %D%/puts.c %D%/remove.c \
	%D%/rename.c %D%/rewind.c %D%/scanf.S %D%/setbuf.c %D%/setvbuf.c %D%/siprintf.S %D%/siscanf.S \
	%D%/sleep.c %D%/sniprintf.S %D%/snprintf.S %D%/sprintf.S %D%/sscanf.S %D%/stdio.c %D%/strcat.c \
	%D%/strchr.c %D%/strcmp.c %D%/strcpy.c %D%/strcspn.c %D%/strlen.c %D%/strncat.c %D%/strncmp.c \
	%D%/strncpy.c %D%/strpbrk.c %D%/strrchr.c %D%/strspn.c %D%/strxfrm.c %D%/tmpfile.c \
	%D%/tmpnam.c %D%/ungetc.c %D%/usleep.c %D%/vfiprintf.c %D%/vfiscanf.c %D%/vfprintf.c \
	%D%/vfscanf.c %D%/viprintf.c %D%/viscanf.c %D%/vprintf.c %D%/vscanf.c %D%/vsiprintf.c \
	%D%/vsiscanf.c %D%/vsniprintf.c %D%/vsnprintf.c %D%/vsprintf.c %D%/vsscanf.c \
	%D%/stack_reg_va.S %D%/spu_clock_svcs.c %D%/spu_clock_stop.c %D%/spu_timer_flih.S \
	%D%/spu_timer_slih.c %D%/spu_timer_slih_reg.c %D%/spu_timer_svcs.c \
	%D%/spu_timer_stop.c %D%/spu_timer_free.c %D%/spu_timebase.c %D%/fdopen.c

if HAVE_SPU_EA
libc_a_SOURCES += \
	%D%/calloc_ea.c %D%/free_ea.c %D%/malloc_ea.c %D%/memchr_ea.c %D%/memcmp_ea.c \
	%D%/memcpy_ea.c %D%/memmove_ea.c %D%/memset_ea.c %D%/mmap_ea.c %D%/mremap_ea.c %D%/msync_ea.c \
	%D%/munmap_ea.c %D%/posix_memalign_ea.c %D%/realloc_ea.c %D%/strcat_ea.c %D%/strchr_ea.c \
	%D%/strcmp_ea.c %D%/strcpy_ea.c %D%/strcspn_ea.c %D%/strlen_ea.c %D%/strncat_ea.c %D%/strncmp_ea.c \
	%D%/strncpy_ea.c %D%/strpbrk_ea.c %D%/strrchr_ea.c %D%/strspn_ea.c %D%/strstr_ea.c %D%/read_ea.c \
	%D%/pread_ea.c %D%/readv_ea.c %D%/write_ea.c %D%/pwrite_ea.c %D%/writev_ea.c %D%/spu-mcount.S \
	%D%/spu-gmon.c
endif
