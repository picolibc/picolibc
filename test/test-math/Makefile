all: 	test-clog.h \
	test-cos.h \
	test-sin.h

test-clog.h: gen-clog.5c gen-complex.5c gen-test.5c
	nickle $< > $@

test-cos.h: gen-cos.5c gen-test.5c
	nickle $< > $@

test-sin.h: gen-sin.5c gen-test.5c
	nickle $< > $@
