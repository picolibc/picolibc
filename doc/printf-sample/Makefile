CFLAGS=-Os \
    -march=armv7-m \
    --specs=picolibc.specs \
    --oslib=semihost \
    --crt0=hosted \
    -Wl,--defsym=__flash=0 \
    -Wl,--defsym=__flash_size=0x00200000 \
    -Wl,--defsym=__ram=0x20000000 \
    -Wl,--defsym=__ram_size=0x200000

QEMU=qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -nographic -kernel

DCFLAGS=$(CFLAGS) -DPICOLIBC_DOUBLE_PRINTF_SCANF
FCFLAGS=$(CFLAGS) -DPICOLIBC_FLOAT_PRINTF_SCANF
LCFLAGS=$(CFLAGS) -DPICOLIBC_LONG_LONG_PRINTF_SCANF
ICFLAGS=$(CFLAGS) -DPICOLIBC_INTEGER_PRINTF_SCANF
MCFLAGS=$(CFLAGS) -DPICOLIBC_MINIMAL_PRINTF_SCANF

CC=arm-none-eabi-gcc

LOG=printf.log printf-float.log printf-long-long.log printf-integer.log printf-minimal.log

ALL=printf.elf printf-float.elf printf-long-long.elf printf-integer.elf printf-minimal.elf

all: $(LOG)

printf.log: printf.elf
	(arm-none-eabi-size $^; $(QEMU) $^) > $@

printf.elf: printf.c
	$(CC) $(DCFLAGS) -o $@ $^

printf-float.log: printf-float.elf
	(arm-none-eabi-size $^; $(QEMU) $^) > $@

printf-float.elf: printf.c
	$(CC) $(FCFLAGS) -o $@ $^

printf-long-long.log: printf-long-long.elf
	(arm-none-eabi-size $^; $(QEMU) $^) > $@

printf-long-long.elf: printf.c
	$(CC) $(LCFLAGS) -o $@ $^

printf-integer.log: printf-integer.elf
	(arm-none-eabi-size $^; $(QEMU) $^) > $@

printf-integer.elf: printf.c
	$(CC) $(ICFLAGS) -o $@ $^

printf-minimal.log: printf-minimal.elf
	(arm-none-eabi-size $^; $(QEMU) $^) > $@

printf-minimal.elf: printf.c
	$(CC) $(MCFLAGS) -o $@ $^

clean:
	rm -f $(ALL) $(LOG)
