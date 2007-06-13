## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

AM_CCASFLAGS = $(INCLUDES)

lib_a_SOURCES = clearerr.c fclose.c feof.c ferror.c fflush.c \
	fgetc.c fgetpos.c fgets.c fileno.c fiprintf.c fiscanf.c fopen.c \
	fprintf.c fputc.c fputs.c fread.c freopen.c fscanf.c fseek.c \
	fsetpos.c ftell.c fwrite.c getc.c getchar.c gets.c iprintf.c \
	iscanf.c memcpy.c memmove.c memset.c mktemp.c perror.c printf.c \
	putc.c putchar.c puts.c remove.c rename.c rewind.c scanf.c \
	setbuf.c setvbuf.c siprintf.c siscanf.c sleep.c sniprintf.c \
	snprintf.c sprintf.c sscanf.c stdio.c strcat.c strchr.c strcmp.c \
	strcpy.c strcspn.c strlen.c strncat.c strncmp.c strncpy.c strpbrk.c \
	strrchr.c strspn.c strxfrm.c tmpfile.c tmpnam.c ungetc.c usleep.c \
	vfiprintf.c vfiscanf.c vfprintf.c vfscanf.c viprintf.c viscanf.c \
	vprintf.c vscanf.c vsiprintf.c vsiscanf.c vsniprintf.c vsnprintf.c \
	vsprintf.c vsscanf.c

lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../.. 
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

