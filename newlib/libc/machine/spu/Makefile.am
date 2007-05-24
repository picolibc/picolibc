## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

AM_CCASFLAGS = $(INCLUDES)

lib_a_SOURCES = setjmp.S memcpy.c memmove.c memset.c strcat.c strchr.c strcmp.c strcpy.c strcspn.c strlen.c strncat.c strncmp.c strncpy.c strpbrk.c strrchr.c strspn.c strxfrm.c \
	stdio.c clearerr.c feof.c ferror.c fileno.c fopen.c fclose.c freopen.c \
	fflush.c fseek.c ftell.c rewind.c fgetpos.c fsetpos.c fread.c fwrite.c \
	getc.c putc.c fgetc.c fgets.c fputc.c fputs.c ungetc.c \
	fprintf.c fscanf.c vfprintf.c vfscanf.c \
	perror.c gets.c getchar.c putchar.c puts.c \
	printf.c scanf.c vprintf.c vscanf.c \
	setbuf.c setvbuf.c tmpfile.c \
	sprintf.c sscanf.c snprintf.c vsscanf.c vsnprintf.c vsprintf.c \
	remove.c rename.c tmpnam.c \
	iprintf.c fiprintf.c siprintf.c sniprintf.c viprintf.c vfiprintf.c vsiprintf.c vsniprintf.c \
	iscanf.c fiscanf.c siscanf.c viscanf.c vfiscanf.c vsiscanf.c

lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../.. 
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

