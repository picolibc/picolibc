volatile int a = 42;
int main (void) {
  return __builtin_expect(a, 1);
}
