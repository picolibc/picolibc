
#ifndef _CYGSERVER_SHM_
#define _CYGSERVER_SHM_
class transport_layer_base *create_server_transport();

/* the base class uses AF_UNIX sockets,but other classes are possible. */
class transport_layer_base
{
  public:
    virtual void listen ();
    virtual class transport_layer_base * accept ();
    virtual void close ();
    virtual ssize_t read (char *buf, size_t len);
    virtual ssize_t write (char *buf, size_t len);
    virtual bool connect();
    virtual void impersonate_client ();
    virtual void revert_to_self ();
    transport_layer_base ();

  private:
    /* for socket based communications */
    int fd;
    struct sockaddr sockdetails;
    int sdlen;
    transport_layer_base (int newfd);
};

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

#endif /* _CYGSERVER_SHM_ */
