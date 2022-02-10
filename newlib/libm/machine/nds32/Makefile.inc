%C%_src =
if HAS_NDS32_FPU_SP
%C%_src += %D%/wf_sqrt.S
endif

if HAS_NDS32_FPU_DP
%C%_src += %D%/w_sqrt.S
endif

libm_a_SOURCES += $(%C%_src)
