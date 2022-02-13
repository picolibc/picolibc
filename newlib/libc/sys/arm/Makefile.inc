AM_CPPFLAGS_%C% = -I$(srcdir)/libc/machine/arm

libc_a_SOURCES += %D%/access.c %D%/aeabi_atexit.c
if MAY_SUPPLY_SYSCALLS
libc_a_SOURCES += %D%/libcfunc.c %D%/trap.S %D%/syscalls.c
endif
