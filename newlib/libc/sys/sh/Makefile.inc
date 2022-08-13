if MAY_SUPPLY_SYSCALLS
libc_a_SOURCES += %D%/syscalls.c %D%/trap.S %D%/creat.c %D%/ftruncate.c %D%/truncate.c
endif
