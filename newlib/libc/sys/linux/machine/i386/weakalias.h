#define weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));

#define weak_extern(symbol) _weak_extern (symbol)
#define _weak_extern(symbol) asm (".weak " #symbol);

#define weak_function __attribute__ ((weak))

