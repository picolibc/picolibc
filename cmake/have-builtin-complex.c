static double _Complex test(double r, double i) { return __builtin_complex(r, i); }
int main(void) { test(1.0, 2.0); return 0; }
