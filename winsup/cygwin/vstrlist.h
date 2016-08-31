/* vstrlist.h: class vstrlist

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus

struct allocator_interface
{
  virtual void * alloc (size_t) = 0;
  virtual void free (void *) = 0;
};


/* The allocated_type makes sure to use the free () method of the
   same allocator_interface than the alloc () method was used of.

   Stores the allocator_interface address before the real object,
   to hide it from (construction & destruction of) real object.  */
class allocated_type
{
  union allocator_store
  {
    allocator_interface * allocator_;
    char alignment_[8];

    union pointer
    {
      void            * vptr;
      allocator_store * real;
    };
  };

public:
  void * operator new (size_t class_size, allocator_interface & allocator)
  {
    allocator_store::pointer astore;
    astore.vptr = allocator.alloc (sizeof (allocator_store) + class_size);
    astore.real->allocator_ = &allocator;
    ++ astore.real;
    return astore.vptr;
  }

  void operator delete (void * p)
  {
    allocator_store::pointer astore;
    astore.vptr = p;
    -- astore.real;
    astore.real->allocator_->free (astore.vptr);
  }
};


/* Double linked list of char arrays, each being a string buffer,
   which's final buffer size and initial string content is defined
   by a NULL terminated variable argument list of STRING+LEN pairs,
   where each STRING (up to LEN) is concatenated for the initial
   string buffer content, and each LEN is added to the final size
   of the allocated string buffer.
   If LEN is -1, strlen(STRING) is used for LEN.

   Needs:
     An implementation of the allocator_interface.

   Provides:
     iterator:
       short name for the string_iterator
     string_iterator:
       provides readonly access via member methods:
	 string (): readonly string buffer
	 stringlength (): length (readonly) of initial string
     buffer_iterator:
       extends string_iterator
       provides writeable access via member methods:
	 buffer (): writeable string buffer
	 bufferlength (): length (readonly) of allocated buffer

   Usage sample:
     char * text = "snipe";
     vstrlist l;
     l.appendv (text, 4, text+3, 2, "", 2, NULL);
     buffer_iterator it (l.begin ());
     strcpy (it->buffer () + it->stringlength (), "ts");
     printf ("Sample result is: '%s'", it->string ());
   Sample result is: 'snippets' */
class vstrlist
{
public:
  class member
    : public allocated_type
  {
    friend class vstrlist;
    friend class string_iterator;

    member * prev_;
    member * next_;
    size_t   bufferlength_;
    size_t   stringlength_;
    char     buffer_[1]; /* we always have space for the trailing zero */

    /* no copy, just swap */
    member (member const &);
    member & operator = (member const &);

    /* anchor */
    void * operator new (size_t class_size, allocator_interface & allocator)
    {
      return allocated_type::operator new (class_size, allocator);
    }

    /* anchor */
    member ()
      : allocated_type ()
      , prev_ (this)
      , next_ (this)
      , bufferlength_ (0)
      , stringlength_ (0)
      , buffer_ ()
    {}

    /* entry: determine memory size from args */
    void * operator new (size_t class_size, allocator_interface & allocator,
			 char const * part0, va_list parts)
    {
      char const * part = part0;
      va_list partsdup;
      va_copy (partsdup, parts);
      while (part)
	{
	  int partlen = va_arg (partsdup, int);
	  if (partlen < 0)
	    partlen = strlen (part);
	  class_size += partlen;
	  part = va_arg (partsdup, char const *);
	}
      va_end (partsdup);

      return allocated_type::operator new (class_size, allocator);
    }

    /* entry: instantly insert into list */
    member (member * before, char const * part0, va_list parts)
      : allocated_type ()
      , prev_ (NULL)
      , next_ (NULL)
      , bufferlength_ (0)
      , stringlength_ ()
      , buffer_ ()
    {
      prev_ = before->prev_;
      next_ = before;

      prev_->next_ = this;
      next_->prev_ = this;

      char * dest = buffer_;
      char const * part = part0;
      va_list partsdup;
      va_copy (partsdup, parts);
      while (part)
	{
	  int partlen = va_arg (partsdup, int);
	  if (partlen < 0)
	    {
	      char * old = dest;
	      dest = stpcpy (old, part);
	      partlen = dest - old;
	    }
	  else
	    dest = stpncpy (dest, part, partlen);
	  bufferlength_ += partlen;
	  part = va_arg (partsdup, const char *);
	}
      va_end (partsdup);
      *dest = (char)0;
      stringlength_ = dest - buffer_;
      if (bufferlength_ > stringlength_)
	memset (++dest, 0, bufferlength_ - stringlength_);
    }

    /* remove entry from list */
    ~member ()
    {
      member * next = next_;
      member * prev = prev_;
      if (next)
	next->prev_ = prev;
      if (prev)
	prev->next_ = next;
      prev_ = NULL;
      next_ = NULL;
    }

  public:
    member const * next () const { return next_; }
    member       * next ()       { return next_; }
    member const * prev () const { return next_; }
    member       * prev ()       { return next_; }

    /* readonly access */
    char const * string () const { return buffer_; }
    size_t stringlength () const { return stringlength_; }

    /* writeable access */
    char       * buffer ()       { return buffer_; }
    size_t bufferlength ()       { return bufferlength_; }
  };

  /* readonly access */
  class string_iterator
  {
    friend class vstrlist;
    friend class buffer_iterator;

    member * current_;

    string_iterator ();

    string_iterator (member * current)
      : current_ (current)
    {}

  public:
    string_iterator (string_iterator const & rhs)
      : current_ (rhs.current_)
    {}

    string_iterator & operator = (string_iterator const & rhs)
    {
      current_ = rhs.current_;
      return *this;
    }

    string_iterator & operator ++ ()
    {
      current_ = current_->next ();
      return *this;
    }

    string_iterator operator ++ (int)
    {
      string_iterator ret (*this);
      current_ = current_->next ();
      return ret;
    }

    string_iterator & operator -- ()
    {
      current_ = current_->prev ();
      return *this;
    }

    string_iterator operator -- (int)
    {
      string_iterator ret (*this);
      current_ = current_->prev ();
      return ret;
    }

    bool operator == (string_iterator const & rhs) const
    {
      return current_ == rhs.current_;
    }

    bool operator != (string_iterator const & rhs) const
    {
      return current_ != rhs.current_;
    }

    /* readonly member access */
    member const & operator *  () const { return *current_; }
    member const * operator -> () const { return  current_; }

    void remove ()
    {
      member * old = current_;
      ++ *this;
      delete old;
    }
  };

  /* writeable access */
  class buffer_iterator
    : public string_iterator
  {
  public:
    explicit /* can be used with vstrlist.begin () */
    buffer_iterator (string_iterator const & begin)
      : string_iterator (begin)
    {}

    buffer_iterator (buffer_iterator const & rhs)
      : string_iterator (rhs)
    {}

    buffer_iterator & operator = (buffer_iterator const & rhs)
    {
      string_iterator::operator = (rhs);
      return *this;
    }

    /* writeable member access */
    member & operator *  () const { return *current_; }
    member * operator -> () const { return  current_; }
  };

private:
  allocator_interface & allocator_;
  member              * anchor_;

  /* not without an allocator */
  vstrlist ();

  /* no copy, just swap () */
  vstrlist (vstrlist const &);
  vstrlist & operator = (vstrlist const &);

public:
  /* iterator is the string_iterator */
  typedef class string_iterator iterator;

  iterator  begin () { return iterator (anchor_->next ()); }
  iterator  end   () { return iterator (anchor_         ); }
  iterator rbegin () { return iterator (anchor_->prev ()); }
  iterator rend   () { return iterator (anchor_         ); }

  vstrlist (allocator_interface & a)
    : allocator_ (a)
    , anchor_ (NULL) /* exception safety */
  {
    anchor_ = new (allocator_) member ();
  }

  ~vstrlist ()
  {
    if (anchor_ != NULL)
      {
	for (iterator it = begin (); it != end (); it.remove ());
	delete anchor_;
      }
  }

  void swap (vstrlist & that)
  {
    allocator_interface & a = allocator_;
    member              * m = anchor_;
    allocator_ = that.allocator_;
    anchor_    = that.anchor_;
    that.allocator_ = a;
    that.anchor_    = m;
  }

  string_iterator appendv (char const * part0, va_list parts)
  {
    member * ret = new (allocator_, part0, parts)
		       member (anchor_, part0, parts);
    return string_iterator (ret);
  }

  string_iterator appendv (char const * part0, ...)
  {
    va_list parts;
    va_start (parts, part0);
    string_iterator ret = appendv (part0, parts);
    va_end (parts);
    return ret;
  }
};

#endif /* __cplusplus */
