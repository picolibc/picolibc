
This code is a utility function I was considering adding to Mingw32. The
Microsoft versions of argc, argv construction use quotes and backslashes
to allow the user to pass arguments containing spaces (or quotes) to 
programs they invoke. The rules are

 - Arguments containing spaces must be enclosed in quotes.
 - A quote can be passed by preceeding it with a backslash.
 - Backslashes immediately preceeding a quote must be doubled to avoid
   escaping the quote.

Thus an argument like:

 -D="Foo Bar\\"

needs to be mangled as:

 "-D\"Foo Bar\\\\\""

in order to get to the program as what was intended above.

The fix_argv set of functions is meant to be used with spawnv and the
like to allow a program to set up an argv array for the spawned program
and have that array duplicated *exactly* in the spawned program, no
matter what it contains (it also quotes 'globbing' characters like *
and ?, so it does not matter if the destination has globbing turned on
or not; it might be a reasonable extension to allow a flag to allow
globbing characters to pass through unmolested, but they would still
be quoted if the string contained whitespace).

The reason for writing this came up because of problems with arguments
like -DBLAH="Foo Bar" to GCC (define BLAH as a preprocessor constant
being the string "Foo Bar", including the quotes). Because GCC simply
passes the argument directly to CPP (the preprocessor) it had to be
escaped *twice*:

 "-DBLAH=\"\\\"Foo Bar\\\"\""

This would reach GCC as

 -DBLAH="\"Foo Bar\""

And that would reach CPP as the desired

 -DBLAH="Foo Bar"

One level of quoting and escaping is to be expected (although MS's
standard is, arguably, not very good), but forcing the user to know
how many different programs the argument is going to pass through,
and perform double quoting and escaping, seems unreasonable. If
GCC and friends all used fix_argv (they use their own version of
it now) then the original argument could be

 "-DBLAH=\"Foo Bar\""

And that would work fine, no matter how many different tools it
passed through.

The only basic limitation with this code is that it assumes that all
the spawned programs use Microsoft-type escaping when interpreting
their command line. Most programs on Win32 machines do (anything
compiled with Mingw32 will).

For now, this code has been relegated to 'sample' status. If you want
to use it, feel free (it is public domain after all).

Colin.

P.S. Just out of interest you might try writing your own little program
     to look at the interaction of wildcards and quotes. Use the glob.exe
     program in ../globbing and see what it does with

      glob "foo*.txt"

     even if there are files foo.txt and foobar.txt in the same directory.

     Note that

      del "My *.txt"

     works (i.e. it deletes all files starting with My<space>). This could
     not be done unless del does globbing *after* processing escapes and
     quotes, which is not the way it seems to work normally (again see
     the glob example).

