## Process this file with automake to generate Makefile.in

if HAVE_POSIX_DIR
POSIX_SUBDIR = posix
endif

if HAVE_SIGNAL_DIR
SIGNAL_SUBDIR = signal
endif

if HAVE_SYSCALL_DIR
SYSCALLS_SUBDIR = syscalls
endif

if HAVE_UNIX_DIR
UNIX_SUBDIR = unix
endif

if HAVE_STDIO_DIR
STDIO_SUBDIR = stdio
endif

if HAVE_STDIO64_DIR
STDIO64_SUBDIR = stdio64
endif

if ENABLE_NEWLIB_ICONV
NEWLIB_ICONV_DIRS = iconv
NEWLIB_ICONV_LIBS = iconv/ces/lib.a iconv/ccs/lib.a iconv/lib/lib.a
else
NEWLIB_ICONV_DIRS =
NEWLIB_ICONV_LIBS =
endif

if HAVE_XDR_DIR
XDR_SUBDIR = xdr
endif

SUBDIRS = argz stdlib ctype search $(STDIO_SUBDIR) $(STDIO64_SUBDIR) string $(SIGNAL_SUBDIR) time locale reent \
	errno misc $(UNIX_SUBDIR) $(POSIX_SUBDIR) $(SYSCALLS_SUBDIR) $(NEWLIB_ICONV_DIRS) \
	$(XDR_SUBDIR) ssp
if HAVE_SYS_DIR
SUBDIRS += $(SYS_DIR)
endif
# NB: Machine dir must come last so it can override common objects when creating
# libc.a below.
if HAVE_LIBC_MACHINE_DIR
SUBDIRS += $(LIBC_MACHINE_DIR)
endif
# NB: This must come last to avoid parallel build issues with current lib.a
# accumulation logic.
SUBDIRS += .

noinst_LIBRARIES = libc.a
SUBLIBS = \
	argz/lib.a \
	stdlib/lib.a \
	ctype/lib.a \
	search/lib.a \
	$(LIBC_STDIO_LIB) \
	$(LIBC_STDIO64_LIB) \
	$(LIBC_XDR_LIB) \
	string/lib.a \
	$(LIBC_SIGNAL_LIB) \
	time/lib.a \
	locale/lib.a \
	reent/lib.a \
	errno/lib.a \
	misc/lib.a \
	ssp/lib.a \
	$(LIBC_UNIX_LIB) \
	$(LIBC_POSIX_LIB) \
	$(LIBC_SYSCALL_LIB) \
        $(NEWLIB_ICONV_LIBS) \
	$(LIBC_MACHINE_LIB) \
	$(LIBC_SYS_LIB)

libc.a: $(SUBLIBS)
	rm -f $@
	rm -rf tmp
	mkdir tmp
	cd tmp; \
	 for i in $(SUBLIBS); do \
	   $(AR) x ../$$i; \
	 done; \
	$(AR) $(AR_FLAGS) ../$@ *.o
	$(RANLIB) $@
	rm -rf tmp

$(SUBLIBS): ; @true
