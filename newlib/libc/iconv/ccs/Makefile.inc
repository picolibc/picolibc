if ENABLE_NEWLIB_ICONV

## The following interfaces are EL/IX level 2
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/ccsbi.c \
	%D%/iso_8859_10.c %D%/iso_8859_13.c %D%/iso_8859_14.c %D%/iso_8859_15.c \
	%D%/iso_8859_1.c %D%/iso_8859_2.c %D%/iso_8859_3.c %D%/iso_8859_4.c \
	%D%/iso_8859_5.c %D%/iso_8859_6.c %D%/iso_8859_7.c %D%/iso_8859_8.c \
	%D%/iso_8859_9.c %D%/iso_8859_11.c \
	%D%/win_1250.c %D%/win_1252.c %D%/win_1254.c \
	%D%/win_1256.c %D%/win_1258.c %D%/win_1251.c \
	%D%/win_1253.c %D%/win_1255.c %D%/win_1257.c \
	%D%/koi8_r.c %D%/koi8_u.c %D%/koi8_ru.c %D%/koi8_uni.c \
	%D%/iso_ir_111.c \
	%D%/big5.c \
	%D%/cp775.c %D%/cp850.c %D%/cp852.c %D%/cp855.c %D%/cp866.c \
	%D%/jis_x0212_1990.c %D%/jis_x0201_1976.c %D%/jis_x0208_1990.c %D%/ksx1001.c \
	%D%/cns11643_plane1.c %D%/cns11643_plane2.c %D%/cns11643_plane14.c
endif

include %D%/binary/Makefile.inc

endif
