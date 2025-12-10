#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static const char s_testData[] = "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890";

static const char s_testFmt[] = "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "%s";
static
void Test5 ( unsigned loops )
{
  assert( strlen( s_testData ) == 100 );
  assert( strlen( s_testFmt  ) == 102 );

  // Compile this code with -fno-builtin.

  char buffer[300];

  for ( unsigned i = 0; i < loops; ++i )
  {
    sprintf( buffer, s_testFmt, s_testData );
  }
}

int
main(int argc, char **argv)
{
    unsigned loops = 0;

    if (argc >= 2)
        loops = atoi(argv[1]);
    if (loops == 0)
        loops = 1000;
    printf("loops: %u\n", loops);
    Test5(loops);
    exit(0);
}
