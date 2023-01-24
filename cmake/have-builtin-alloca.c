static int foo(void * i __attribute__((unused))) { return 0; }
int main(void) { return foo(__builtin_alloca(1)); }
