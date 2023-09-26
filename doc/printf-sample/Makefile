#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2023 Keith Packard
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#

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
