//
// C++ test of a dll which contains a C++ class.
//

#include <stdlib.h>
#include <stdio.h>

// Interface of class.
#include "silly.h"

#ifdef	DERIVED_TEST
// Here is a derived class too.
class CMoreSilly : public CSilly
{
  public:
	CMoreSilly (char* szNewName) : CSilly (szNewName) {};
	~CMoreSilly ();

	WhatsYourName();
};

CMoreSilly::
~CMoreSilly ()
{
	printf ("In CMoreSilly \"%s\" destructor!\n", szName);
}

CMoreSilly::
WhatsYourName ()
{
	printf ("I'm more silly and my name is \"%s\"\n", szName);
}
#endif

int
main ()
{
	CSilly*	psilly = new CSilly("silly");

	psilly->WhatsYourName();
	psilly->Poke();		// Poke him, he should say "Ouch!"
	psilly->Stab(4);	// Stab him four times he should say "Ugh!!!!"

	delete psilly;

#ifdef DERIVED_TEST
	psilly = new CMoreSilly("more silly");
	psilly->WhatsYourName();
	psilly->Stab(5);
	delete psilly;
#endif

	return 0;
}

