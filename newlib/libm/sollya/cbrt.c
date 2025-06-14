#include <math.h>
#include <stdio.h>

double
my_cbrt(double x)
{
    return x * 0x1.21db04db601c7p2 + pow(x, 0x1p1) * (-0x1.965c4d4f4d03fp3) + pow(x, 0x1.8p1) * 0x1.5fc6984dbe32p4 + pow(x, 0x1p2) * (-0x1.5e28a6c7eebb7p4) + pow(x, 0x1.4p2) * 0x1.74349ad72e3e5p3 + pow(x, 0x1.8p2) * (-0x1.4807782df657ep1);
}

double lo = 0x1p-1;
double hi = 0x1p0;

int main(void)
{
    double x;
    for (x = lo; x < hi; x += (hi-lo)/20) {
        double libm = cbrt(x);
        double my = my_cbrt(x);

        printf("x: %a libm: %a my: %a delta %a\n",
               x, libm, my, fabs(libm - my));
    }
}
