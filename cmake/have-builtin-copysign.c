static int foo(double i __attribute__((unused))) { return 0; }
int main(void) { return foo(__builtin_copysign(42, 42)); }
