static int __attribute__((no_builtin)) foo(int x) { return x + 1; }
volatile int aa = 42;
int main(void) { return foo(aa); }
