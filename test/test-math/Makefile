.SUFFIXES:
.SUFFIXES: .5c .h

NICKLE=nickle

REAL_TRIG = \
	test-acos.h \
	test-acosh.h \
	test-acospi.h \
	test-asin.h \
	test-asinh.h \
	test-asinpi.h \
	test-atan.h \
	test-atan2.h \
	test-atanh.h \
	test-atanpi.h \
	test-atan2pi.h \
	test-cos.h \
	test-cosh.h \
	test-cospi.h \
	test-sin.h \
	test-sinh.h \
	test-sinpi.h \
	test-tan.h \
	test-tanh.h \
	test-tanpi.h

REAL_EXP = \
	test-compoundn.h \
	test-exp.h \
	test-exp10.h \
	test-exp2.h \
	test-expm1.h \
	test-log.h \
	test-log10.h \
	test-log1p.h \
	test-log2.h \
	test-pow.h \
	test-pown.h

REAL_NARROW = \
	test-fadd.h \
	test-fsub.h \
	test-fmul.h \
	test-fdiv.h \
	test-fsqrt.h \
	test-ffma.h \
	test-dadd.h \
	test-dsub.h \
	test-dmul.h \
	test-ddiv.h \
	test-dsqrt.h \
	test-dfma.h

REAL_OTHER = \
	test-cbrt.h \
	test-ceil.h \
	test-erf.h \
	test-erfc.h \
	test-floor.h \
	test-hypot.h \
	test-j0.h \
	test-j1.h \
	test-jn.h \
	test-lgamma.h \
	test-remainder.h \
	test-remquo.h \
	test-round.h \
	test-scalbn.h \
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
	$(REAL_TRIG) $(REAL_EXP) $(REAL_NARROW) $(REAL_OTHER) \
	$(COMPLEX_TRIG) $(COMPLEX_EXP) $(COMPLEX_OTHER)

all: $(TEST_FILES) .clang-format-ignore

$(TEST_FILES): test-complex.5c test-float.5c math-test-copyright

.DELETE_ON_ERROR:

.5c.h:
	($(NICKLE) $< && cat math-test-copyright) > $@

.clang-format-ignore: Makefile
	for i in '!**/*.[ch]' $(TEST_FILES); do echo "$$i"; done > $@

CC_80=x86_64-linux-gnu-gcc

CC_128=aarch64-linux-gnu-gcc

gen-narrowing-f-64: gen-narrowing.c
	$(CC) -g -O0 -o $@ $< -lm

gen-narrowing-f-80: gen-narrowing.c
	$(CC_80) -g -O0 -o $@ -DWIDE_LD $< -lm

gen-narrowing-f-128: gen-narrowing.c
	$(CC_128) -g -O0 -o $@ -DWIDE_LD $< -lm

gen-narrowing-d-80: gen-narrowing.c
	$(CC_80) -g -O0 -o $@ -DNARROW_D -DWIDE_LD $< -lm

gen-narrowing-d-128: gen-narrowing.c
	$(CC_128) -g -O0 -o $@ -DNARROW_D -DWIDE_LD $< -lm

$(REAL_NARROW): narrow-stamp test-narrow.5c

GEN_NARROWING_F=gen-narrowing-f-64 gen-narrowing-f-80 gen-narrowing-f-128
GEN_NARROWING_D=gen-narrowing-d-80 gen-narrowing-d-128
GEN_NARROWING=$(GEN_NARROWING_F) $(GEN_NARROWING_D)

narrow-stamp: $(GEN_NARROWING) make-narrowing
	./make-narrowing
	touch $@

clean:
	rm -f $(GEN_NARROWING) narrow-stamp
	rm -f $(TEST_FILES)
	rm -f test-[df]*-inc.5c
