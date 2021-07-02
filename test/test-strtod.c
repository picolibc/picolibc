#include <stdlib.h>

int main(void)
{
    double x = strtod("1e2", NULL);
    return x > 1 ? 0 : 1;
}
