/* cygserver.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _CYGSERVER_TRANSPORT_PIPES_
#define _CYGSERVER_TRANSPORT_PIPES_
/* Named pipes based transport, for security on NT */
class transport_layer_pipes : public transport_layer_base
{
  public:
    virtual void listen ();
    virtual class transport_layer_pipes * accept ();
    virtual void close ();
    virtual ssize_t read (char *buf, size_t len);
    virtual ssize_t write (char *buf, size_t len);
    virtual bool connect();
    virtual void impersonate_client ();
    virtual void revert_to_self ();
    transport_layer_pipes ();

  private:
    /* for pipe based communications */
    void init_security ();
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sec_none_nih, sec_all_nih;
    char pipe_name [MAX_PATH];
    HANDLE pipe;
    bool inited;
    transport_layer_pipes (HANDLE new_pipe);
};
#endif /* _CYGSERVER_TRANSPORT_PIPES_ */
