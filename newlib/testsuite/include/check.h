#define CHECK(a) { \
  if (!(a)) \
    { \
      printf ("Failed " #a " in <%s> at line %d\n", __FILE__, __LINE__); \
      abort(); \
    } \
}
