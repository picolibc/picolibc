
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

main ()
{
	PRINTDLG	pd;
	DOCINFO		di;
	char*		szMessage;

	memset (&pd, 0, sizeof(PRINTDLG));
	memset (&di, 0, sizeof(DOCINFO));

	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = "Test";

	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_PAGENUMS | PD_RETURNDC;
	pd.nFromPage = 1;
	pd.nToPage = 1;
	pd.nMinPage = 1;
	pd.nMaxPage = 1;

	szMessage = 0;

	if (PrintDlg (&pd))
	{
		if (pd.hDC)
		{
			if (StartDoc (pd.hDC, &di) != SP_ERROR)
			{
				StartPage (pd.hDC);

				TextOut (pd.hDC, 0, 0, "Hello, printer!", 15);

				EndPage (pd.hDC);

				EndDoc (pd.hDC);

				szMessage = "Printed.";
			}
			else
			{
				szMessage = "Could not start document.";
			}
		}
		else
		{
			szMessage = "Could not create device context.";
		}
	}
	else
	{
		szMessage = "Canceled or printer could not be setup.";
	}

	if (szMessage)
	{
		MessageBox (NULL, szMessage, "Print Test", MB_OK);
	}

	return 0;
}
