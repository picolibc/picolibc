libc_a_SOURCES += \
	%D%/memcpy.S %D%/memset.S %D%/setjmp.S %D%/strcmp.S %D%/strcpy.S \
	%D%/strlen.S %D%/strncpy.S

if XTENSA_ESP32_PSRAM_CACHE_FIX
libc_a_CPPFLAGS_%C% = \
	-DXTENSA_ESP32_PSRAM_CACHE_FIX
endif
