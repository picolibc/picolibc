static __inline__ long int
random (void)
{
  extern int rand(void);

  return rand ();
}

static __inline__ void
srandom (unsigned int seed)
{
  void srand(unsigned int seed);

  srand (seed);
}
