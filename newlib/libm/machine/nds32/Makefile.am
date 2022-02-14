## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = -I $(abs_newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

src =

if HAS_NDS32_FPU_SP
src += wf_sqrt.S
endif

if HAS_NDS32_FPU_DP
src += w_sqrt.S
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src)
lib_a_CFLAGS = $(AM_CFLAGS)
