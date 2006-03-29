
/*
** Copyright (C) 1991 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

char *sys_errlist[] = {
  "no error",
  "invalid function",
  "file not found",
  "path not found",
  "too many open files",
  "access denied",
  "invalid file handle",
  "arena trashed",
  "not enough memory",
  "invalid block",
  "no environment",
  "no format",
  "invalid access code",
  "invalid data",
  "undefined",
  "invalid drive",
  "attempt to remove current directory",
  "not same device",
  "no more files"
};

int sys_nerr= sizeof(sys_errlist) / sizeof(char *);

