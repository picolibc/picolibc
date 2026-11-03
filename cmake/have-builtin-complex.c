static int
foo(double _Complex i __attribute__((unused)))
{
    return 0;
}
int
main(void)
{
    return foo(__builtin_complex(1.0, 2.0));
}
