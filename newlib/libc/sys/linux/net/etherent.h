/* Because the `ethers' lookup does not fit so well in the scheme we
   define a dummy struct here which helps us to use the available
   functions.  */
struct etherent
{
  const char *e_name;
  struct ether_addr e_addr;
};

