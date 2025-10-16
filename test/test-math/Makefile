.SUFFIXES:
.SUFFIXES: .5c .h

NICKLE=nickle

REAL_TRIG = \
	test-acos.h \
	test-acosh.h \
	test-asin.h \
	test-asinh.h \
	test-atan.h \
	test-atan2.h \
	test-atanh.h \
	test-cos.h \
	test-cosh.h \
	test-sin.h \
	test-sinh.h \
	test-tan.h \
	test-tanh.h

REAL_EXP = \
	test-exp.h \
	test-exp10.h \
	test-exp2.h \
	test-expm1.h \
	test-log.h \
	test-log10.h \
	test-log1p.h \
	test-log2.h \
	test-pow.h

REAL_OTHER = \
	test-cbrt.h \
	test-erf.h \
	test-erfc.h \
	test-hypot.h \
	test-j0.h \
	test-j1.h \
	test-jn.h \
	test-lgamma.h \
	test-sqrt.h \
	test-tgamma.h

COMPLEX_TRIG = \
	test-cacos.h \
	test-cacosh.h \
	test-casin.h \
	test-casinh.h \
	test-catan.h \
	test-catanh.h \
	test-ccos.h \
	test-ccosh.h \
	test-csin.h \
	test-csinh.h \
	test-ctan.h \
	test-ctanh.h

COMPLEX_EXP = \
	test-cexp.h \
	test-clog.h \
	test-cpow.h

COMPLEX_OTHER = \
	test-csqrt.h

TEST_FILES = \
	$(REAL_TRIG) $(REAL_EXP) $(REAL_OTHER) \
	$(COMPLEX_TRIG) $(COMPLEX_EXP) $(COMPLEX_OTHER)

all: $(TEST_FILES)

$(TEST_FILES): test-complex.5c test-float.5c

.DELETE_ON_ERROR:

.5c.h:
	($(NICKLE) $< && cat math-test-copyright) > $@

clean:
	rm -f $(TEST_FILES)
