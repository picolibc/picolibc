#ifndef _CYGSERVER_TRANSPORT_
#define _CYGSERVER_TRANSPORT_
class transport_layer_base *create_server_transport();

/* the base class does nothing. */
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
};

#endif /* _CYGSERVER_TRANSPORT_ */
