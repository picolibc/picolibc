/* no-op function as entry point for applications built between
   2010-09-11 and 2011-03-16.  That's the timeframe of _feinitialise
   being called from mainCRTStartup in crt0.o. */
void _feinitialise (void)
{}
