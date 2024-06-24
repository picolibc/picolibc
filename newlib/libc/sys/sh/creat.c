extern int
_creat (const char *path, int mode);

int
creat (const char *path, int mode)
{
  return _creat (path, mode);
}
