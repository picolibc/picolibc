libc_a_SOURCES += \
	%D%/ctype_.c \
	%D%/isalnum.c \
	%D%/isalpha.c \
	%D%/iscntrl.c \
	%D%/isdigit.c \
	%D%/islower.c \
	%D%/isupper.c \
	%D%/isprint.c \
	%D%/ispunct.c \
	%D%/isspace.c \
	%D%/isxdigit.c \
	%D%/tolower.c \
	%D%/toupper.c

## The following handles EL/IX level 2 functions
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/categories.c \
	%D%/isalnum_l.c \
	%D%/isalpha_l.c \
	%D%/isascii.c \
	%D%/isascii_l.c \
	%D%/isblank.c \
	%D%/isblank_l.c \
	%D%/iscntrl_l.c \
	%D%/isdigit_l.c \
	%D%/islower_l.c \
	%D%/isupper_l.c \
	%D%/isprint_l.c \
	%D%/ispunct_l.c \
	%D%/isspace_l.c \
	%D%/iswalnum.c \
	%D%/iswalnum_l.c \
	%D%/iswalpha.c \
	%D%/iswalpha_l.c \
	%D%/iswblank.c \
	%D%/iswblank_l.c \
	%D%/iswcntrl.c \
	%D%/iswcntrl_l.c \
	%D%/iswctype.c \
	%D%/iswctype_l.c \
	%D%/iswdigit.c \
	%D%/iswdigit_l.c \
	%D%/iswgraph.c \
	%D%/iswgraph_l.c \
	%D%/iswlower.c \
	%D%/iswlower_l.c \
	%D%/iswprint.c \
	%D%/iswprint_l.c \
	%D%/iswpunct.c \
	%D%/iswpunct_l.c \
	%D%/iswspace.c \
	%D%/iswspace_l.c \
	%D%/iswupper.c \
	%D%/iswupper_l.c \
	%D%/iswxdigit.c \
	%D%/iswxdigit_l.c \
	%D%/isxdigit_l.c \
	%D%/jp2uc.c \
	%D%/toascii.c \
	%D%/toascii_l.c \
	%D%/tolower_l.c \
	%D%/toupper_l.c \
	%D%/towctrans.c \
	%D%/towctrans_l.c \
	%D%/towlower.c \
	%D%/towlower_l.c \
	%D%/towupper.c \
	%D%/towupper_l.c \
	%D%/wctrans.c \
	%D%/wctrans_l.c \
	%D%/wctype.c \
	%D%/wctype_l.c
endif

LIBC_CHEWOUT_FILES += \
	%D%/isalnum.def \
	%D%/isalpha.def \
	%D%/isascii.def \
	%D%/isblank.def \
	%D%/iscntrl.def \
	%D%/isdigit.def \
	%D%/islower.def \
	%D%/isprint.def \
	%D%/ispunct.def \
	%D%/isspace.def \
	%D%/isupper.def \
	%D%/iswalnum.def \
	%D%/iswalpha.def \
	%D%/iswblank.def \
	%D%/iswcntrl.def \
	%D%/iswctype.def \
	%D%/iswdigit.def \
	%D%/iswgraph.def \
	%D%/iswlower.def \
	%D%/iswprint.def \
	%D%/iswpunct.def \
	%D%/iswspace.def \
	%D%/iswupper.def \
	%D%/iswxdigit.def \
	%D%/isxdigit.def \
	%D%/toascii.def \
	%D%/tolower.def \
	%D%/toupper.def \
	%D%/towctrans.def \
	%D%/towlower.def \
	%D%/towupper.def \
	%D%/wctrans.def \
	%D%/wctype.def

LIBC_CHAPTERS += %D%/ctype.tex
