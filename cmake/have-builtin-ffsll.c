static int foo(long long i __attribute__((unused))) { return 0; }
int main(void) { return foo(__builtin_ffsll((long long)42)); }
