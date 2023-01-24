static int foo(long double i __attribute__((unused))) { return 0; }
int main(void) { return foo(__builtin_copysignl((long double)42, (long double) 42)); }
