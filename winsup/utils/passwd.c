/* passwd.c: Changing passwords and managing account information

   Copyright 1999 Cygnus Solutions.

   Written by Corinna Vinschen <corinna.vinschen@cityweb.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>

#include <windows.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmapibuf.h>

#define USER_PRIV_ADMIN		 2

#define UF_LOCKOUT            0x00010

char *myname;

int
eprint (int with_name, const char *fmt, ...)
{
  va_list ap;

  if (with_name)
    fprintf(stderr, "%s: ", myname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fprintf(stderr, "\n");
  return 1;
}

int
EvalRet (int ret, const char *user)
{
  switch (ret)
    {
    case NERR_Success:
      return 0;

    case ERROR_ACCESS_DENIED:
      if (! user)
        eprint (0, "You may not change password expiry information.");
      else
        eprint (0, "You may not change the password for %s.", user);
      break;

      eprint (0, "Bad password: Invalid.");
      break;

    case NERR_PasswordTooShort:
      eprint (0, "Bad password: Too short.");
      break;

    case NERR_UserNotFound:
      eprint (1, "unknown user %s", user);
      break;

    case ERROR_INVALID_PASSWORD:
    case NERR_BadPassword:
      eprint (0, "Incorrect password for %s.", user);
      eprint (0, "The password for %s is unchanged.", user);
      break;

    default:
      eprint (1, "unrecoverable error %d", ret);
      break;
    }
  return 1;
}

PUSER_INFO_3
GetPW (const char *user)
{
  WCHAR name[512];
  DWORD ret;
  PUSER_INFO_3 ui;

  MultiByteToWideChar (CP_ACP, 0, user, -1, name, 512);
  ret = NetUserGetInfo (NULL, name, 3, (LPBYTE *) &ui);
  return EvalRet (ret, user) ? NULL : ui;
}

int
ChangePW (const char *user, const char *oldpwd, const char *pwd)
{
  WCHAR name[512], oldpass[512], pass[512];
  DWORD ret;

  MultiByteToWideChar (CP_ACP, 0, user, -1, name, 512);
  MultiByteToWideChar (CP_ACP, 0, pwd, -1, pass, 512);
  if (! oldpwd)
    {
      USER_INFO_1003 ui;

      ui.usri1003_password = pass;
      ret = NetUserSetInfo (NULL, name, 1003, (LPBYTE) &ui, NULL);
    }
  else
    {
      MultiByteToWideChar (CP_ACP, 0, oldpwd, -1, oldpass, 512);
      ret = NetUserChangePassword (NULL, name, oldpass, pass);
    }
  if (! EvalRet (ret, user))
    {
      eprint (0, "Password changed.");
    }
  return ret;
}

void
PrintPW (PUSER_INFO_3 ui)
{
  time_t t = time (NULL) - ui->usri3_password_age;
  int ret;
  PUSER_MODALS_INFO_0 mi;

  printf ("Account disabled : %s", (ui->usri3_flags & UF_ACCOUNTDISABLE)
                                ? "yes\n" : "no\n");
  printf ("Password required: %s", (ui->usri3_flags & UF_PASSWD_NOTREQD)
                                ? "no\n" : "yes\n");
  printf ("Password expired : %s", (ui->usri3_password_expired)
                                ? "yes\n" : "no\n");
  printf ("Password changed : %s", ctime(&t));
  ret = NetUserModalsGet (NULL, 0, (LPBYTE *) &mi);
  if (! ret)
    {
      if (mi->usrmod0_max_passwd_age == TIMEQ_FOREVER
          || ui->usri3_priv == USER_PRIV_ADMIN)
        mi->usrmod0_max_passwd_age = 0;
      if (mi->usrmod0_min_passwd_age == TIMEQ_FOREVER
          || ui->usri3_priv == USER_PRIV_ADMIN)
        mi->usrmod0_min_passwd_age = 0;
      if (mi->usrmod0_force_logoff == TIMEQ_FOREVER
          || ui->usri3_priv == USER_PRIV_ADMIN)
        mi->usrmod0_force_logoff = 0;
      if (ui->usri3_priv == USER_PRIV_ADMIN)
        mi->usrmod0_min_passwd_len = 0;
      printf ("Max. password age %ld days\n",
              mi->usrmod0_max_passwd_age / ONE_DAY);
      printf ("Min. password age %ld days\n",
              mi->usrmod0_min_passwd_age / ONE_DAY);
      printf ("Force logout after %ld days\n",
              mi->usrmod0_force_logoff / ONE_DAY);
      printf ("Min. password length: %ld\n",
              mi->usrmod0_min_passwd_len);
    }
}

int
SetModals (int xarg, int narg, int iarg, int Larg)
{
  int ret;
  PUSER_MODALS_INFO_0 mi;

  ret = NetUserModalsGet (NULL, 0, (LPBYTE *) &mi);
  if (! ret)
    {
      if (xarg == 0)
	mi->usrmod0_max_passwd_age = TIMEQ_FOREVER;
      else if (xarg > 0)
	mi->usrmod0_max_passwd_age = xarg * ONE_DAY;

      if (narg == 0)
	{
	  mi->usrmod0_min_passwd_age = TIMEQ_FOREVER;
	  mi->usrmod0_password_hist_len = 0;
	}
      else if (narg > 0)
	mi->usrmod0_min_passwd_age = narg * ONE_DAY;

      if (iarg == 0)
	mi->usrmod0_force_logoff = TIMEQ_FOREVER;
      else if (iarg > 0)
	mi->usrmod0_force_logoff = iarg * ONE_DAY;

      if (Larg >= 0)
	mi->usrmod0_min_passwd_len = Larg;

      ret = NetUserModalsSet (NULL, 0, (LPBYTE) mi, NULL);
      NetApiBufferFree (mi);
    }
  return EvalRet (ret, NULL);
}

int
usage ()
{
  fprintf (stderr, "usage: %s [name]\n", myname);
  fprintf (stderr, "       %s [-L maxlen] [-x max] [-n min] [-i inact]\n",
           myname);
  fprintf (stderr, "       %s {-l|-u|-S} name\n", myname);
  return 2;
}

int
main (int argc, char **argv)
{
  char *c;
  char user[64], oldpwd[64], newpwd[64];
  int ret = 0;
  int cnt = 0;
  int opt;
  int Larg = -1;
  int xarg = -1;
  int narg = -1;
  int iarg = -1;
  int lopt = 0;
  int uopt = 0;
  int Sopt = 0;
  PUSER_INFO_3 ui, li;

 if ((myname = strrchr (argv[0], '/'))
      || (myname = strrchr (argv[0], '\\')))
    ++myname;
  else
    myname = argv[0];
  c = strrchr (myname, '.');
  if (c)
    *c = '\0';

  while ((opt = getopt (argc, argv, "L:x:n:i:luS")) != EOF)
    switch (opt)
      {
      case 'x':
	if ((xarg = atoi (optarg)) < 0 || xarg > 999)
	  return eprint (1, "Maximum password age must be between 0 and 999.");
	if (narg >= 0 && xarg < narg)
	  return eprint (1, "Maximum password age must be greater than "
	                    "minimum password age.");
        break;

      case 'n':
	if ((narg = atoi (optarg)) < 0 || narg > 999)
	  return eprint (1, "Minimum password age must be between 0 and 999.");
	if (xarg >= 0 && narg > xarg)
	  return eprint (1, "Minimum password age must be less than "
	                    "maximum password age.");
        break;

      case 'i':
	if ((iarg = atoi (optarg)) < 0 || iarg > 999)
	  return eprint (1, "Force logout time must be between 0 and 999.");
        break;

      case 'L':
	if ((Larg = atoi (optarg)) < 0 || Larg > LM20_PWLEN)
	  return eprint (1, "Minimum password length must be between "
	                    "0 and %d.", LM20_PWLEN);
        break;

      case 'l':
	if (xarg >= 0 || narg >= 0 || iarg >= 0 || Larg >= 0 || uopt || Sopt)
	  return usage ();
	lopt = 1;
        break;

      case 'u':
	if (xarg >= 0 || narg >= 0 || iarg >= 0 || Larg >= 0 || lopt || Sopt)
	  return usage ();
	uopt = 1;
        break;

      case 'S':
	if (xarg >= 0 || narg >= 0 || iarg >= 0 || Larg >= 0 || lopt || uopt)
	  return usage ();
	Sopt = 1;
        break;

      default:
        return usage ();
      }
  if (Larg >= 0 || xarg >= 0 || narg >= 0 || iarg >= 0)
    {
      if (optind < argc)
        return usage ();
      return SetModals (xarg, narg, iarg, Larg);
    }

  strcpy (user, optind >= argc ? getlogin () : argv[optind]);

  li = GetPW (getlogin ());
  if (! li)
    return 1;

  ui = GetPW (user);
  if (! ui)
    return 1;

  if (lopt || uopt || Sopt)
    {
      if (li->usri3_priv != USER_PRIV_ADMIN)
        return eprint (0, "You have no maintenance privileges.");
      if (lopt)
        {
	  if (ui->usri3_priv == USER_PRIV_ADMIN)
	    return eprint (0, "You may not lock an administrators account.");
          ui->usri3_flags |= UF_ACCOUNTDISABLE;
        }
      if (uopt)
        ui->usri3_flags &= ~UF_ACCOUNTDISABLE;
      if (lopt || uopt)
	{
          ret = NetUserSetInfo (NULL, ui->usri3_name, 3, (LPBYTE) ui, NULL);
          return EvalRet (ret, NULL);
	}
      // Sopt
      PrintPW (ui);
      return 0;
    }

  if (li->usri3_priv != USER_PRIV_ADMIN && strcmp (getlogin (), user))
    return eprint (0, "You may not change the password for %s.", user);

  eprint (0, "Enter the new password (minimum of 5, maximum of 8 characters).");
  eprint (0, "Please use a combination of upper and lower case letters and numbers.");

  oldpwd[0] = '\0';
  if (li->usri3_priv != USER_PRIV_ADMIN)
    {
      strcpy (oldpwd, getpass ("Old password: "));
      if (ChangePW (user, oldpwd, oldpwd))
        return 1;
    }

  do
    {
      strcpy (newpwd, getpass ("New password: "));
      if (strcmp (newpwd, getpass ("Re-enter new password: ")))
        eprint (0, "Password is not identical.");
      else if (! ChangePW (user, *oldpwd ? oldpwd : NULL, newpwd))
        ret = 1;
      if (! ret && cnt < 2)
        eprint (0, "Try again.");
    }
  while (! ret && ++cnt < 3);
  return ! ret;
}
