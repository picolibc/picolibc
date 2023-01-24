static int foo(long i __attribute__((unused))) { return 0; }
int main(void) { return foo(__builtin_ffsl((long)42)); }
