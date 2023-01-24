struct test { int part: 24; } __attribute__((packed));
static unsigned int foo (const struct test *p) { return p->part; }
int main() { struct test x = { 12 }; return foo(&x); }
