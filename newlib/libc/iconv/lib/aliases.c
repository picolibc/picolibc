/*
 * Copyright (c) 2003, Artem B. Bityuckiy, SoftMine Corporation.
 * Rights transferred to Franklin Electronic Publishers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <string.h>
#include <stdlib.h>
#include <reent.h>
#include "local.h"

/*
 * strnstr - locate a substring in a fixed-size string.
 *
 * PARAMETERS:
 *    _CONST char *haystack - the string in which to search.
 *    _CONST char *needle   - the string which to search.
 *    int length            - the maximum 'haystack' string length.
 *
 * DESCRIPTION:
 *    The  strstr() function finds the first occurrence of the substring 
 *    'needle'in the string 'haystack'. At most 'length' bytes is searched.
 *
 * RETURN:
 *    Returns a pointer to the beginning of the substring, or NULL if the 
 *    substring was not found.
 */
static char *
_DEFUN(strnstr, (haystack, needle, length),
                _CONST char *haystack _AND
                _CONST char *needle   _AND
                int length)
{
  int i;
  _CONST char *max = haystack + length;

  if (*haystack == '\0')
      return *needle == '\0' ? (char *)haystack : NULL;

  while (haystack < max)
  {
      i = 0;
      while (1)
      {
          if (needle[i] == '\0')
              return (char *)haystack;
          if (needle[i] != haystack[i++])
              break;
      }
      haystack += 1;
  }
  return (char *)NULL;
}

/*
 * canonical_form - canonize 'str'.
 *
 * PARAMETERS:
 *    struct _reent *rptr - reent structure of curent thread/process.
 *    _CONST char *str - string to canonize. 
 *
 * DESCRIPTION:
 *     Converts all letters to small and substitute all '-' by '_'.
 *
 * RETURN:
 *     Returns canonical form of 'str' if success, NULL if failure.
 */
static _CONST char *
_DEFUN(canonical_form, (rptr, str), 
                       struct _reent *rptr _AND
                       _CONST char *str)
{
    char *p, *p1;

    if (str == NULL || (p = p1 = _strdup_r(rptr, str)) == NULL)
        return NULL;

    for (; *str; str++, p++)
    {
        if (*str == '-')
            *p = '_';
        else
            *p = tolower(*str);
    }

    return (_CONST char *)p1; 
}

/*
 * find_alias - find "official" name by alias. 
 *
 * PARAMETERS:
 *    struct _reent *rptr - reent structure of curent thread/process.
 *    _CONST char *alias  - alias by which "official" name should be found.
 *    _CONST char *table  - aliases table.
 *    int len             - aliases table length.
 *
 * DESCRIPTION:
 *     'table' contains the list of names and their aliases. "Official" 
 *      names go first, e.g.:
 *
 *     Official_name1 alias11 alias12 alias1N
 *     Official_name2 alias21 alias22 alias2N
 *     Official_nameM aliasM1 aliasM2 aliasMN
 *
 *     If line begins with backspace it is considered as the continuation of
 *     previous line.
 *
 * RETURN:
 *     Returns pointer to "official" name if success, NULL if failure.
 */
static _CONST char *
_DEFUN(find_alias, (rptr, alias, table, len),
       struct _reent *rptr _AND
       _CONST char *alias  _AND
       _CONST char *table  _AND
       int len)
{
    _CONST char *end;
    _CONST char *p;
    _CONST char *candidate;
    int l = strlen(alias);
    _CONST char *ptable = table;
    _CONST char *table_end = table + len;

    if (table == NULL || alias == NULL || *table == '\0' || *alias == '\0')
        return NULL;

search_again:
    if (len < l)
        return NULL;
    
    if ((p = strnstr(ptable, alias, len)) == NULL)
        return NULL;
    
    /* Check that substring is segregated by '\n', '\t' or ' ' */
    if (!((p == table || *(p-1) == ' ' || *(p-1) == '\n' || *(p-1) == '\t') &&
          (p+l == table_end || *(p+l) == ' ' || *(p+l) == '\n' || *(p+l) == '\t')))
    {
        ptable = p + l;
        len -= table - p;
        goto search_again;
    }

    while(!(--p < table || (*p == '\n' && *(p+1) != ' ' && *(p+1) != '\t')));

    if (*(++p) == '#')
        return NULL;

    for (end = p + 1; *end != '\t' && *end != ' ' && *end != '\n' && *end != '\0'; end++);

    return _strndup_r(rptr, p, (size_t)(end - p));
}

/*
 * _iconv_construct_filename -- constructs full file name from it's 
 * path and name
 *
 * PARAMETERS:
 *    struct _reent *rptr - reent structure of current thread/process.  
 *    _CONST char *path - path to file.
 *    _CONST char *file - the name of file.
 *
 * DESCRIPTION:
 *     Function concatenates 'path' and 'file'.
 *     'path' and 'name' shouldn't be NULL.
 *
 * RETURN:
 *     The pointer to full file name if success, NULL if faulture.
 */
char *
_DEFUN(_iconv_construct_filename, (rptr, path, file),
       struct _reent *rptr _AND
       _CONST char *path   _AND
       _CONST char *file)
{
    int len1, len2;
    char *fname;
    
    if (path == NULL || file == NULL)
        return NULL;
  
    len1 = strlen(path);
    len2 = strlen(file);

    if ((fname = _malloc_r(rptr, len1 + len2 + 2 /* '/' + '\0' */)) == NULL)
        return NULL;

    memcpy(fname, path, len1);
    if (fname[len1 - 1] != '/')
        fname[len1++] = '/';
    memcpy(fname + len1, file, len2);
    fname[len1 + len2] = '\0';
    
    return fname;
}

/*
 * _iconv_resolve_alias - resolves "official" name by given alias. 
 *
 * PARAMETERS:
 *    struct _reent *rptr    - reent structure of curent thread/process.
 *    _CONST char *alias     - alias to resolve.
 *    _CONST char *bialiases - aliases table.
 *    int cf                 - canonize flag.
 *    _CONST char *fname     - name of external file with list uf aliases.
 *
 * DESCRIPTION:
 *     Tries to find 'alias' in aliases list ('bialiases'). If not 
 *     found, searches at 'fname' file. 'cf' flag shows if 'alias' should be 
 *     canonized before searching. Both 'bialiases' and 'fname' can't be NULL
 *     pointers.
 * 
 * RETURN:
 *     Returns pointer to name found if success, NULL if failure.
 */
char *
_DEFUN(_iconv_resolve_alias, (rptr, alias, bialiases, cf, fname), 
       struct _reent *rptr _AND
       _CONST char *alias _AND
       _CONST char *bialiases _AND
       int cf _AND
       _CONST char *fname)
{
    _CONST char *name = alias;
    _CONST char *ptr = alias;
    _CONST char *table;
    _iconv_fd_t desc;

    /* Alias shouldn't contain whitespaces, '\n' and '\r' symbols */ 
    while (*ptr)
        if (*ptr == ' ' || *ptr == '\r' || *ptr++ == '\n')
            return NULL;
    
    if (cf && (name = canonical_form(rptr, alias)) == NULL)
        return NULL;

    if ((alias = find_alias(rptr, name, (char *)bialiases, 
                            strlen(bialiases))) != NULL)
        goto free_and_return;
    
    if (_iconv_load_file(rptr, fname, &desc) != 0)
        goto free_and_return;

    alias = find_alias(rptr, name, desc.mem, desc.len);

close_free_and_return:    
    _iconv_unload_file(rptr, &desc);
free_and_return:
    if (cf)
        _free_r(rptr, (_VOID_PTR)name);
    return (char*)alias;
}

/*
 * _iconv_resolve_cs_name - resolves convrter's "official" name by given alias. 
 *
 * PARAMETERS:
 *     struct _reent *rptr - reent structure of curent thread/process.
 *     _CONST char *cs     - charset alias to resolve.
 *     _CONST char *path   - external file with aliases list.
 *
 * DESCRIPTION: 
 * First, tries to find 'cs' among built-in aliases. If not fount, tries to 
 * find it external file.
 *
 * RETURN: Official name if found, NULL else.
 */
char *
_DEFUN(_iconv_resolve_cs_name, (rptr, cs, path), 
                            struct _reent *rptr _AND
                            _CONST char *cs    _AND
                            _CONST char *path)
{
  char *fname, *p;

  if (path == NULL || *path == (_CONST char)0)
      if ((path = _getenv_r(rptr, "NLSPATH")) == NULL || *path == '\0')
          path = NLS_DEFAULT_NLSPATH;
  
  fname = _iconv_construct_filename(rptr, path, ICONV_ALIASES_FNAME);
  if (fname == NULL)
      return NULL;
  
  p = (char *)_iconv_resolve_alias(rptr, cs, _iconv_builtin_aliases, 
                                1, (_CONST char *)fname);
  if (fname != NULL)
      _free_r(rptr, (_VOID_PTR)fname);
  return p;
}

