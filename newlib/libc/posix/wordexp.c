/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
#ifndef _NO_WORDEXP

#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/queue.h>

#include <wordexp.h>
#include "wordexp2.h"

#define MAXLINELEN 500

/* Note: This implementation of wordexp requires a version of bash
   that supports the --wordexp and --protected arguments to be present
   on the system.  It does not support the WRDE_UNDEF flag. */
int
wordexp(const char *__restrict words, wordexp_t *__restrict pwordexp, int flags)
{
  FILE *f = NULL;
  FILE *f_err = NULL;
  char tmp[MAXLINELEN];
  int i = 0;
  int offs = 0;
  char *iter;
  pid_t pid;
  int num_words = 0;
  int num_bytes = 0;
  int fd[2];
  int fd_err[2];
  int err = WRDE_NOSPACE;
  ext_wordv_t *wordv = NULL;
  char *eword;
  struct ewords_entry *entry;

  if (pwordexp == NULL)
    {
      return WRDE_NOSPACE;
    }

  if (flags & WRDE_REUSE)
    wordfree(pwordexp);

  if ((flags & WRDE_APPEND) == 0)
    {
      pwordexp->we_wordc = 0;
      pwordexp->we_wordv = NULL;
    }

  if (flags & WRDE_DOOFFS)
    {
      offs = pwordexp->we_offs;

      if (pwordexp->we_wordv)
        wordv = WE_WORDV_TO_EXT_WORDV(pwordexp->we_wordv);
      wordv = (ext_wordv_t *)realloc(wordv, sizeof(ext_wordv_t) + (offs + pwordexp->we_wordc) * sizeof(char *));
      if (!wordv)
        return err;
      if (!pwordexp->we_wordv)
        SLIST_INIT(&wordv->list);
      pwordexp->we_wordv = wordv->we_wordv;

      for (i = 0; i < offs; i++)
        pwordexp->we_wordv[i] = NULL;
    }

  if (pipe(fd))
    return err;
  if (pipe(fd_err))
    {
      close(fd[0]);
      close(fd[1]);
      return err;
    }
  pid = fork();

  if (pid == -1)
    {
      /* In "parent" process, but fork failed */
      close(fd_err[0]);
      close(fd_err[1]);
      close(fd[0]);
      close(fd[1]);
      return err;
    }
  else if (pid > 0)
    {
      /* In parent process. */

      /* Close write end of parent's pipe. */
      close(fd[1]);
      close(fd_err[1]);

      /* f_err is the standard error from the shell command. */
      if (!(f_err = fdopen(fd_err[0], "r")))
        goto cleanup;

      /* Check for errors. */
      if (fgets(tmp, MAXLINELEN, f_err))
        {
          if (strstr(tmp, "EOF"))
            err = WRDE_SYNTAX;
          else if (strstr(tmp, "`\n'") || strstr(tmp, "`|'")
                   || strstr(tmp, "`&'") || strstr(tmp, "`;'")
                   || strstr(tmp, "`<'") || strstr(tmp, "`>'")
                   || strstr(tmp, "`('") || strstr(tmp, "`)'")
                   || strstr(tmp, "`{'") || strstr(tmp, "`}'"))
            err = WRDE_BADCHAR;
          else if (strstr(tmp, "command substitution"))
            err = WRDE_CMDSUB;
          else
            err = WRDE_SYNTAX;

          if (flags & WRDE_SHOWERR)
            {
              fputs(tmp, stderr);
              while(fgets(tmp, MAXLINELEN, f_err))
                fputs(tmp, stderr);
            }

          goto cleanup;
        }

      /* f is the standard output from the shell command. */
      if (!(f = fdopen(fd[0], "r")))
        goto cleanup;

      /* Get number of words expanded by shell. */
      if (!fgets(tmp, MAXLINELEN, f))
        goto cleanup;

      if((iter = strchr(tmp, '\n')))
          *iter = '\0';

      num_words = atoi(tmp);

      if (pwordexp->we_wordv)
        wordv = WE_WORDV_TO_EXT_WORDV(pwordexp->we_wordv);
      wordv = (ext_wordv_t *)realloc(wordv, sizeof(ext_wordv_t) + (offs + pwordexp->we_wordc + num_words) * sizeof(char *));
      if (!wordv)
        return err;
      if (!pwordexp->we_wordv)
        SLIST_INIT(&wordv->list);
      pwordexp->we_wordv = wordv->we_wordv;

      /* Get number of bytes required for storage of all num_words words. */
      if (!fgets(tmp, MAXLINELEN, f))
        goto cleanup;

      if((iter = strchr(tmp, '\n')))
          *iter = '\0';

      num_bytes = atoi(tmp);

      if (!(entry = (struct ewords_entry *)malloc(sizeof(struct ewords_entry) + num_bytes + num_words)))
        goto cleanup;
      SLIST_INSERT_HEAD(&wordv->list, entry, next);

      /* Get expansion from the shell output. */
      if (!fread(entry->ewords, 1, num_bytes + num_words, f))
        goto cleanup;
      entry->ewords[num_bytes + num_words] = 0;

      /* Store each entry in pwordexp's we_wordv vector. */
      eword = entry->ewords;
      for(i = 0; i < num_words; i++, eword = iter)
        {
          if (!eword)
            break;
          pwordexp->we_wordv[offs + pwordexp->we_wordc + i] = eword;
          if ((iter = strchr(eword, '\n')))
            *iter++ = '\0';
        }

      pwordexp->we_wordv[offs + pwordexp->we_wordc + i] = NULL;
      pwordexp->we_wordc += num_words;
      if (i == num_words)
        err = WRDE_SUCCESS;

cleanup:
      if (f)
        fclose(f);
      else
        close(fd[0]);
      if (f_err)
        fclose(f_err);
      else
        close(fd_err[0]);

      /* Wait for child to finish. */
      waitpid (pid, NULL, 0);

      return err;
    }
  else
    {
      /* In child process. */

      /* Close read end of child's pipe. */
      close(fd[0]);
      close(fd_err[0]);

      /* Pipe standard output to parent process via fd. */
      if (fd[1] != STDOUT_FILENO)
        {
          if (dup2(fd[1], STDOUT_FILENO) == -1)
            _exit(EXIT_FAILURE);
          /* fd[1] no longer required. */
          close(fd[1]);
        }

      /* Pipe standard error to parent process via fd_err. */
      if (fd_err[1] != STDERR_FILENO)
        {
          if (dup2(fd_err[1], STDERR_FILENO) == -1)
            _exit(EXIT_FAILURE);
          /* fd_err[1] no longer required. */
          close(fd_err[1]);
        }

      if (flags & WRDE_NOCMD)
        execl("/bin/bash", "bash", "--protected", "--wordexp", words, (char *)0);
      else
        execl("/bin/bash", "bash", "--wordexp", words, (char *)0);
      _exit(EXIT_FAILURE);
    }
  return WRDE_SUCCESS;
}
#endif /* !_NO_WORDEXP  */
