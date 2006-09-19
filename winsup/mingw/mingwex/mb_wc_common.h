__declspec(dllimport) unsigned int __lc_codepage;

static inline
unsigned int get_codepage (void)
{
  return __lc_codepage;
}
