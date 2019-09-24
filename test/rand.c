#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

bool
near(double got, double target, double close)
{
	if (got < target - close)
		return false;
	if (got > target + close)
		return false;
	return true;
}

int
main(int argc, char **argv)
{
	int i;
	int ret = 0;
	double s1 = 0;
	double s2 = 0;
#define N	1000000
	for (i = 0; i < N; i++) {
		double d = drand48();
		s1 += d;
		s2 += d*d;
	}
	double mean = s1 / N;
	double stddev = sqrt((N * s2 - s1*s1) / ((double) N * ((double) N - 1)));
	if (!near(mean, .5, .1)) {
		printf("bad mean %g\n", mean);
		ret = 1;
	}
	if (!near(stddev, .28, .1)) {
		printf("bad stddev %g\n", stddev);
		ret = 2;
	}
	fflush(stdout);
	return ret;
}
