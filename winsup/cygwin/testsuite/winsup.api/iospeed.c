#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <windows.h>

int verbose = 0;

void
v(char *fmt, ...)
{
  va_list ap;
  if (!verbose) return;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

#define TSIZE (1024 * 1024 * 16)

unsigned long start_tic;

void
start(FILE *f)
{
  fseek(f, 0, SEEK_SET);
  start_tic = GetTickCount();
}

void
end()
{
  unsigned long end_tic = GetTickCount();
  printf("%6d", end_tic - start_tic);
}

void
test(int linesz, int cr)
{
  FILE *f = fopen("iospeed.dat", "wb");
  char buf[65536];
  int i, fd;

  memset(buf, 'x', linesz);
  buf[linesz-1] = '\n';
  if (cr)
    buf[linesz-2] = '\r';
  for (i=0; i<TSIZE; i += linesz)
    fwrite(buf, 1, linesz, f);
  fclose(f);

  f = fopen("iospeed.dat", "rt");
  fd = fileno(f);

  printf("%6d%6d", linesz, cr);
  for (i=0; i<TSIZE; i+= 65536)
    read(fd, buf, 65536);

  start(f);
  while (getc(f) != EOF);
  end();

  start(f);
  while (fread(buf, 1, 256, f) > 0);
  end();

  start(f);
  while (fgets(buf, 64436, f));
  end();

  f = fopen("iospeed.dat", "rb");
  fd = fileno(f);

  for (i=0; i<TSIZE; i+= 65536)
    read(fd, buf, 65536);

  start(f);
  while (getc(f) != EOF);
  end();

  start(f);
  while (fread(buf, 1, 256, f) > 0);
  end();

  start(f);
  while (fgets(buf, 64436, f));
  end();

  printf("\n");
}

int
main(int argc, char **argv)
{
  if (argc > 1 && strcmp(argv[1],"-v") == 0)
    verbose = 1;

  setbuf(stdout, 0);

  printf("              ----- text -----  ---- binary ----\n");
  printf("linesz    cr  getc fread fgets  getc fread fgets\n");

  test(4, 0);
  test(64, 0);
  test(4096, 0);
  test(4, 1);
  test(64, 1);
  test(4096, 1);

  remove ("iospeed.dat");

  return 0;
}
