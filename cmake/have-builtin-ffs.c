static int foo(int i __attribute__((unused))) { return 0; }
int main(void) { return foo(__builtin_ffs(42)); }
