# Some of the tests in here depend on functionality that only a few ports
# provide.  Disable building it by default until that's figured out.
#check_PROGRAMS += %D%/test
EXTRA_PROGRAMS += %D%/test

%C%_test_SOURCES = \
	%D%/test.c \
	%D%/string.c \
	%D%/convert.c \
	%D%/conv_vec.c \
	%D%/iconv_vec.c \
	%D%/test_is.c \
	%D%/dvec.c \
	%D%/sprint_vec.c \
	%D%/sprint_ivec.c \
	%D%/math2.c \
	%D%/test_ieee.c \
	%D%/math.c \
	%D%/atan2_vec.c \
	%D%/atan2f_vec.c \
	%D%/jn_vec.c \
	%D%/jnf_vec.c \
	%D%/log2_vec.c \
	%D%/log2f_vec.c \
	%D%/yn_vec.c \
	%D%/ynf_vec.c \
	%D%/acos_vec.c \
	%D%/acosf_vec.c \
	%D%/acosh_vec.c \
	%D%/acoshf_vec.c \
	%D%/asin_vec.c \
	%D%/asinf_vec.c \
	%D%/asinh_vec.c \
	%D%/asinhf_vec.c \
	%D%/atan_vec.c \
	%D%/atanf_vec.c \
	%D%/atanh_vec.c \
	%D%/atanhf_vec.c \
	%D%/ceil_vec.c \
	%D%/ceilf_vec.c \
	%D%/cos_vec.c \
	%D%/cosf_vec.c \
	%D%/cosh_vec.c \
	%D%/coshf_vec.c \
	%D%/erf_vec.c \
	%D%/erfc_vec.c \
	%D%/erfcf_vec.c \
	%D%/erff_vec.c \
	%D%/exp_vec.c \
	%D%/expf_vec.c \
	%D%/fabs_vec.c \
	%D%/fabsf_vec.c \
	%D%/floor_vec.c \
	%D%/floorf_vec.c \
	%D%/gamma_vec.c \
	%D%/gammaf_vec.c \
	%D%/j0_vec.c \
	%D%/j0f_vec.c \
	%D%/j1_vec.c \
	%D%/j1f_vec.c \
	%D%/log10_vec.c \
	%D%/log10f_vec.c \
	%D%/log1p_vec.c \
	%D%/log1pf_vec.c \
	%D%/log_vec.c \
	%D%/logf_vec.c \
	%D%/sin_vec.c \
	%D%/sinf_vec.c \
	%D%/sinh_vec.c \
	%D%/sinhf_vec.c \
	%D%/sqrt_vec.c \
	%D%/sqrtf_vec.c \
	%D%/tan_vec.c \
	%D%/tanf_vec.c \
	%D%/tanh_vec.c \
	%D%/tanhf_vec.c \
	%D%/y0_vec.c \
	%D%/y0f_vec.c \
	%D%/y1_vec.c \
	%D%/y1f_vec.c \
	%D%/hypotf_vec.c \
	%D%/hypot_vec.c \
	%D%/fmod_vec.c \
	%D%/fmodf_vec.c
%C%_test_LDADD = $(CRT0) libm.a libc.a
