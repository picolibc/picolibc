
#define	DERIVED_TEST	1

class CSilly
{
  protected:
	char*	szName;

  public:
	CSilly();
	CSilly(char* szName);
#ifdef DERIVED_TEST
	virtual ~CSilly();
#else
	~CSilly();
#endif

	Poke ();
	Stab (int nTimes);
#ifdef DERIVED_TEST
	virtual WhatsYourName ();
#else
	WhatsYourName ();
#endif

};

