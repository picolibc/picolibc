/* pathfinder.h: find one of multiple file names in path list

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "vstrlist.h"

#ifdef __cplusplus

/* Search a list of directory names for first occurrence of a file,
   which's file name matches one out of a list of file names.  */
class pathfinder
{
public:
  typedef vstrlist searchdirlist;
  typedef vstrlist basenamelist;

private:
  pathfinder ();
  pathfinder (pathfinder const &);
  pathfinder & operator = (pathfinder const &);

  basenamelist basenames_;
  size_t       basenames_maxlen_;

  /* Add to searchdirs_ with extra buffer for any basename we may search for.
     This is an optimization for the loops in check_path_access method. */
  searchdirlist searchdirs_;

public:
  ~pathfinder () {}

  /* We need the basenames to search for first, to allow for optimized
     memory allocation of each searchpath + longest basename combination.
     The incoming list of basenames is emptied (ownership take over). */
  pathfinder (allocator_interface & a, basenamelist & basenames)
    : basenames_ (a)
    , basenames_maxlen_ ()
    , searchdirs_(a)
  {
    basenames_.swap(basenames);

    for (basenamelist::buffer_iterator basename (basenames_.begin ());
	 basename != basenames_.end ();
	 ++ basename)
      {
	if (basenames_maxlen_ < basename->bufferlength ())
	  basenames_maxlen_ = basename->bufferlength ();
      }
  }

  void add_searchdir (const char *dir, int dirlen)
  {
      if (dirlen < 0)
	dirlen = strlen (dir);

      if (!dirlen)
	return;

      searchdirs_.appendv (dir, dirlen, "/", 1 + basenames_maxlen_, NULL);
  }

  void add_searchpath (const char *path)
  {
    while (path && *path)
      {
	const char *next = strchr (path, ':');
	add_searchdir (path, next ? next - path : -1);
	path = next ? next + 1 : next;
      }
  }

  void add_envsearchpath (const char *envpath)
    {
      add_searchpath (getenv (envpath));
    }


  /* pathfinder::criterion_interface
     Overload this test method when you need separate dir and basename.  */
  struct criterion_interface
  {
    virtual char const * name () const { return NULL; }

    virtual bool test (searchdirlist::iterator dir,
		       basenamelist::iterator name) const = 0;
  };


  /* pathfinder::simple_criterion_interface
     Overload this test method when you need a single filename.  */
  class simple_criterion_interface
    : public criterion_interface
  {
    virtual bool test (searchdirlist::iterator dir,
		       basenamelist::iterator name) const
    {
      /* Complete the filename path to search for within dir,
	 We have allocated enough memory above.  */
      searchdirlist::buffer_iterator dirbuf (dir);
      memcpy (dirbuf->buffer () + dirbuf->stringlength (),
	      name->string (), name->stringlength () + 1);
      bool ret = test (dirbuf->string ());
      /* reset original dir */
      dirbuf->buffer ()[dirbuf->stringlength ()] = '\0';
      return ret;
    }

  public:
    virtual bool test (const char * filename) const = 0;
  };


  /* pathfinder::path_conv_criterion_interface
     Overload this test method when you need a path_conv. */
  class path_conv_criterion_interface
    : public simple_criterion_interface
  {
    path_conv mypc_;
    path_conv & pc_;
    unsigned opt_;

    /* simple_criterion_interface */
    virtual bool test (const char * filename) const
    {
      pc_.check (filename, opt_);
      return test (pc_);
    }

  public:
    path_conv_criterion_interface (unsigned opt = PC_SYM_FOLLOW)
      : mypc_ ()
      , pc_ (mypc_)
      , opt_ (opt)
    {}

    path_conv_criterion_interface (path_conv & ret, unsigned opt = PC_SYM_FOLLOW)
      : mypc_ ()
      , pc_ (ret)
      , opt_ (opt)
    {}

    virtual bool test (path_conv & pc) const = 0;
  };


  /* pathfinder::exists_and_not_dir
     Test if path_conv argument does exist and is not a directory. */
  struct exists_and_not_dir
    : public path_conv_criterion_interface
  {
    virtual char const * name () const { return "exists and not dir"; }

    exists_and_not_dir (path_conv & pc, unsigned opt = PC_SYM_FOLLOW)
      : path_conv_criterion_interface (pc, opt)
    {}

    /* path_conv_criterion_interface */
    virtual bool test (path_conv & pc) const
    {
      if (pc.exists () && !pc.isdir ())
	return true;

      pc.error = ENOENT;
      return false;
    }
  };


  /* Find the single dir + basename that matches criterion.

     Calls criterion.test method for each registered dir + basename
     until returning true:
       Returns true with found_dir + found_basename set.
     If criterion.test method never returns true:
       Returns false, not modifying found_dir nor found_basename.  */
  bool find (criterion_interface const & criterion,
	     searchdirlist::member const ** found_dir = NULL,
	     basenamelist::member const ** found_basename = NULL)
  {
    char const * critname = criterion.name ();
    for (searchdirlist::iterator dir(searchdirs_.begin ());
	 dir != searchdirs_.end ();
	 ++dir)
      for (basenamelist::iterator name = basenames_.begin ();
	   name != basenames_.end ();
	   ++name)
	if (criterion.test (dir, name))
	  {
	    debug_printf ("(%s), take %s%s", critname,
			  dir->string(), name->string ());
	    if (found_dir)
	      *found_dir = dir.operator -> ();
	    if (found_basename)
	      *found_basename = name.operator -> ();
	    return true;
	  }
	else
	  debug_printf ("not (%s), skip %s%s", critname,
			dir->string(), name->string ());
    return false;
  }
};

#endif /* __cplusplus */
