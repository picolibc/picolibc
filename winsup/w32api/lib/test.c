/* Perform simple test of headers to avoid typos and such */
#define Win32_Winsock
#include <windows.h>

#ifdef __OBJC__
#define BOOL WINBOOL
#endif
#include <windowsx.h>
#include <commctrl.h>
#include <largeint.h>
#include <mmsystem.h>
#include <mciavi.h>
#include <mcx.h>
#include <sql.h>
#include <sqlext.h>
#include <imm.h>
#include <lm.h>
#include <zmouse.h>
#include <scrnsave.h>
#include <cpl.h>
#include <cplext.h>
#include <wincrypt.h>
#include <pbt.h>
#include <wininet.h>
#include <regstr.h>
#include <custcntl.h>

#ifndef __OBJC__
#include <ole2.h>
#include <shlobj.h>
#else
#undef BOOL
#endif

#include <stdio.h>

int main()
{
  return 0;
}
