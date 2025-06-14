CFLAGS=-O0 -g -mfpmath=sse -msse2 -march=core2
OBJS = gamma_32.o gamma_64.o gamma_80.o gamma_128.o

math_test: math_test.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ math_test.o $(OBJS) -lm

clean:
	rm -f math_test math_test.o $(OBJS)

$(OBJS): math2.h math2_inc.h gamma_inc.h
