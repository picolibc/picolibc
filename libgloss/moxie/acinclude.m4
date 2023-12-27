dnl Don't build crt0 for moxiebox, which provides crt0 for us.
AS_CASE([${target}],
  [moxie-*-moxiebox*], [MOXIE_BUILD_CRT0=false],
  [MOXIE_BUILD_CRT0=true])
AM_CONDITIONAL([MOXIE_BUILD_CRT0], [$MOXIE_BUILD_CRT0])
