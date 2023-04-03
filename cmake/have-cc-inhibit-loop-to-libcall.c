__attribute__ ((__optimize__ ("-fno-tree-loop-distribute-patterns")))
static int foo(int x) { return x + 1; }
volatile int aa = 42;
int main(void) { return foo(aa); }
