#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2019 Keith Packard
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

CFLAGS=--oslib=semihost

CC_RISCV=riscv64-unknown-elf-gcc $(CFLAGS) -march=rv32imac -mabi=ilp32
CXX_RISCV=riscv64-unknown-elf-g++ $(CFLAGS) -march=rv32imac -mabi=ilp32

CC_ARM=arm-none-eabi-gcc $(CFLAGS) -march=armv7-m
CXX_ARM=arm-none-eabi-g++ $(CFLAGS) -march=armv7-m

CC_AARCH64=aarch64-none-elf-gcc $(CFLAGS)
CXX_AARCH64=aarch64-none-elf-g++ $(CFLAGS)

all: \
	hello-world-arm.elf hello-world-aarch64.elf \
	hello-world-riscv.elf hello-world-native.elf \
	hello-worldpp-arm.elf hello-worldpp-aarch64.elf \
	hello-worldpp-riscv.elf hello-worldpp-native.elf \
	printf.elf printf-float.elf printf-long.elf \
	printf-int.elf printf-min.elf

hello-world-arm.elf: hello-world.c
	$(CC_ARM) -O2 -o $@ $^ -Wl,-Map=hello-world-arm.map

hello-world-aarch64.elf: hello-world.c
	$(CC_AARCH64) -O2 -o $@ $^ -Wl,-Map=hello-world-aarch64.map

hello-world-riscv.elf: hello-world.c
	$(CC_RISCV) -O2 -o $@ $^ -Wl,-Map=hello-world-riscv.map

hello-world-native.elf: hello-world.c
	cc -O2 -o $@ $^

printf.elf: printf.c
	$(CC_ARM) -Os -o $@ $^ -Wl,-Map=printf.map

printf-float.elf: printf.c
	$(CC_ARM) -Os --printf=f -o $@ $^ -Wl,-Map=printf-float.map

printf-long.elf: printf.c
	$(CC_ARM) -Os --printf=l -o $@ $^ -Wl,-Map=printf-long.map

printf-int.elf: printf.c
	$(CC_ARM) -Os --printf=i -o $@ $^ -Wl,-Map=printf-int.map

printf-min.elf: printf.c
	$(CC_ARM) -Os --printf=m -o $@ $^ -Wl,-Map=printf-min.map

hello-worldpp-native.elf: hello-worldpp.cpp
	c++ -O2 -o $@ $^

hello-worldpp-arm.elf:  hello-worldpp.cpp
	$(CXX_ARM) -O2 -Wl,-Map=hello-worldpp-arm.map -o $@ $^

hello-worldpp-aarch64.elf:  hello-worldpp.cpp
	$(CXX_AARCH64) -O2 -Wl,-Map=hello-worldpp-aarch64.map -o $@ $^

hello-worldpp-riscv.elf:  hello-worldpp.cpp
	$(CXX_RISCV) -O2 -Wl,-Map=hello-worldpp-riscv.map -o $@ $^

clean:
	rm -f *.elf *.map

