volatile int a = 42;
int main (void) {
  return __builtin_have_constant_p(a);
}
