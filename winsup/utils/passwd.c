/* passwd.c: Changing passwords and managing account information

   Copyright 1999, 2000, 2001, 2002 Red Hat, Inc.

   Written by Corinna Vinschen <corinna.vinschen@cityweb.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
#include <wininet.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmapibuf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/cygwin.h>
#include <sys/types.h>
#include <time.h>

#define USER_PRIV_ADMIN		 2

#define UF_LOCKOUT            0x00010

static const char version[] = "$Revision$";
static char *prog_name;

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"inactive", required_argument, NULL, 'i'},
  {"lock", no_argument, NULL, 'l'},
  {"minage", required_argument, NULL, 'n'},
  {"unlock", no_argument, NULL, 'u'},
  {"version", no_argument, NULL, 'v'},
  {"maxage", required_argument, NULL, 'x'},
  {"length", required_argument, NULL, 'L'},
  {"status", no_argument, NULL, 'S'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "L:x:n:i:luShv";

int
eprint (int with_name, const char *fmt, ...)
{
  va_list ap;

  if (with_name)
    fprintf(stderr, "%s: ", prog_name);
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
GetPW (char *user, int print_win_name)
{
  char usr_buf[UNLEN + 1];
  WCHAR name[2 * (UNLEN + 1)];
  DWORD ret;
  PUSER_INFO_3 ui;
  struct passwd *pw;
  char *domain = (char *) alloca (INTERNET_MAX_HOST_NAME_LENGTH + 1);
     
  /* Try getting a Win32 username in case the user edited /etc/passwd */
  if ((pw = getpwnam (user)))
    {
      cygwin_internal (CW_EXTRACT_DOMAIN_AND_USER, pw, domain, usr_buf);
      if (strcasecmp (pw->pw_name, usr_buf))
	{
	  /* Hack to avoid problem with LookupAccountSid after impersonation */
	  if (strcasecmp (usr_buf, "SYSTEM"))
	    {
	      user = usr_buf;
	      if (print_win_name)
		printf ("Windows username : %s\n", user);
	    }
	}
    }
  MultiByteToWideChar (CP_ACP, 0, user, -1, name, 2 * (UNLEN + 1));
  ret = NetUserGetInfo (NULL, name, 3, (LPBYTE *) &ui);
  return EvalRet (ret, user) ? NULL : ui;
}

int
ChangePW (const char *user, const char *oldpwd, const char *pwd, int justcheck)
{
  WCHAR name[2 * (UNLEN + 1)], oldpass[512], pass[512];
  DWORD ret;

  MultiByteToWideChar (CP_ACP, 0, user, -1, name, 2 * (UNLEN + 1));
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
  if (justcheck && ret != ERROR_INVALID_PASSWORD)
    return 0;
  if (! EvalRet (ret, user) && ! justcheck)
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

static void
usage (FILE * stream, int status)
{
  fprintf (stream, ""
  "Usage: %s (-l|-u|-S) [USER]\n"
  "       %s [-i NUM] [-n MINDAYS] [-x MAXDAYS] [-L LEN]\n"
  "\n"
  "User operations:\n"
  " -l, --lock      lock USER's account\n"
  " -u, --unlock    unlock USER's account\n"
  " -S, --status    display password status for USER (locked, expired, etc.)\n"
  "\n"
  "System operations:\n"
  " -i, --inactive  set NUM of days before inactive accounts are disabled\n"
  "                 (inactive accounts are those with expired passwords)\n"
  " -n, --minage    set system minimum password age to MINDAYS\n"
  " -x, --maxage    set system maximum password age to MAXDAYS\n"
  " -L, --length    set system minimum password length to LEN\n"
  "\n"
  "Other options:\n"
  " -h, --help      output usage information and exit\n"
  " -v, --version   output version information and exit\n"
  "", prog_name, prog_name);
  exit (status);
}

static void
print_version ()
{
  const char *v = strchr (version, ':');
  int len;
  if (!v)
    {
      v = "?";
      len = 1;
    }
  else
    {
      v += 2;
      len = strchr (v, ' ') - v;
    }
  printf ("\
%s (cygwin) %.*s\n\
Password Utility\n\
Copyright 1999, 2000, 2001, 2002 Red Hat, Inc.\n\
Compiled on %s\n\
", prog_name, len, v, __DATE__);
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

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];
  else
    prog_name++;
  c = strrchr (prog_name, '.');
  if (c)
    *c = '\0';

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'h':
	usage (stdout, 0);
        break;

      case 'i':
	if ((iarg = atoi (optarg)) < 0 || iarg > 999)
	  return eprint (1, "Force logout time must be between 0 and 999.");
        break;

      case 'l':
	if (xarg >= 0 || narg >= 0 || iarg >= 0 || Larg >= 0 || uopt || Sopt)
	  usage (stderr, 1);
	lopt = 1;
        break;

      case 'n':
	if ((narg = atoi (optarg)) < 0 || narg > 999)
	  return eprint (1, "Minimum password age must be between 0 and 999.");
	if (xarg >= 0 && narg > xarg)
	  return eprint (1, "Minimum password age must be less than "
	                    "maximum password age.");
        break;

      case 'u':
	if (xarg >= 0 || narg >= 0 || iarg >= 0 || Larg >= 0 || lopt || Sopt)
	  usage (stderr, 1);
	uopt = 1;
        break;

      case 'v':
	print_version ();
        exit (0);
        break;

      case 'x':
	if ((xarg = atoi (optarg)) < 0 || xarg > 999)
	  return eprint (1, "Maximum password age must be between 0 and 999.");
	if (narg >= 0 && xarg < narg)
	  return eprint (1, "Maximum password age must be greater than "
	                    "minimum password age.");
        break;

      case 'L':
	if ((Larg = atoi (optarg)) < 0 || Larg > LM20_PWLEN)
	  return eprint (1, "Minimum password length must be between "
	                    "0 and %d.", LM20_PWLEN);
        break;

      case 'S':
	if (xarg >= 0 || narg >= 0 || iarg >= 0 || Larg >= 0 || lopt || uopt)
	  usage (stderr, 1);
	Sopt = 1;
        break;

      default:
        usage (stderr, 1);
      }
  if (Larg >= 0 || xarg >= 0 || narg >= 0 || iarg >= 0)
    {
      if (optind < argc)
        usage (stderr, 1);
      return SetModals (xarg, narg, iarg, Larg);
    }

  strcpy (user, optind >= argc ? getlogin () : argv[optind]);

  li = GetPW (getlogin (), 0);
  if (! li)
    return 1;

  ui = GetPW (user, 1);
  if (! ui)
    return 1;

  if (lopt || uopt || Sopt)
    {
      if (li->usri3_priv != USER_PRIV_ADMIN)
        return eprint (0, "You have no maintenance privileges.");
      if (lopt)
        {
	  if (ui->usri3_priv == USER_PRIV_ADMIN)
	    return eprint (0, "Locking an admin account is disallowed.");
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
      if (ChangePW (user, oldpwd, oldpwd, 1))
        return 1;
    }

  do
    {
      strcpy (newpwd, getpass ("New password: "));
      if (strcmp (newpwd, getpass ("Re-enter new password: ")))
        eprint (0, "Password is not identical.");
      else if (! ChangePW (user, *oldpwd ? oldpwd : NULL, newpwd, 0))
        ret = 1;
      if (! ret && cnt < 2)
        eprint (0, "Try again.");
    }
  while (! ret && ++cnt < 3);
  return ! ret;
}
