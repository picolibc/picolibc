*In compliance with the [GPL-2.0](https://opensource.org/licenses/GPL-2.0) license: I declare that this version of the program contains my modifications, which can be seen through the usual "git" mechanism.*  


2022-12  
Contributor(s):  
Keith Packard  
>math/ld: Add a bunch of double double supportThese are the easy bits: float/int conversions, min, max, and nan.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Fix up lgamma_r names on systems with overlapping float typesCreate correct aliases for lgammal_r, lgamma_r and lgammaf_r on systemswhere those might be fewer than three distinct types.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Fix math aliases for getpayload functionsThese take a const <type> *.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.8' into debianVersion 1.8  
>math/common: Add check_oflowl and check_uflowlCheck over/under flow and set exceptions/errors.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Add __math_denorm helpersThese raise FE_UNDERFLOW and set ERANGE while alsopassing along a denorm value.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-11  
Contributor(s):  
Keith Packard  
Ayke van Laethem  
>math: Make function aliases for correct typesIf float and double are the same size, then alias float functionsto the double names.If double and long double are the same size, then alias double functionsto the long double names.If double is not 64 bits, but long double is, then use the 'double' code forlong double instead.This also gets rid of the separate files containing code for_DOUBLE_IS_32BITS, making things smaller.Signed-off-by: Keith Packard <keithp@keithp.com>  
>avr: work around Clang crashClang has a bug where it will crash for some inputs when built withassertions enabled. While this is a bug in Clang (and not in picolibc),it does reveal a possible issue in picolibc. So I figured I'd send apatch to picolibc as well as fixing it in Clang.Fix to Clang: https://reviews.llvm.org/D138681  
>Switch read/write to posix typesRemove legacy cygwin hacks which set the buffer size and return valuefor read and write to int, switch them to the correct POSIX types,size_t for buffer size and ssize_t for the return value.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib/strtod: strtod_l doesn't use loc in many configsIt's still part of the API though, so just (void) it.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Remove unused 'denorm' in ldtoa.c:e113toeNot sure how this slipped past for so long, but it isn't used in this function.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-10  
Contributor(s):  
Keith Packard  
>Merge tag '1.7.9' into debianVersion 1.7.9  
>Merge i686 and x86_64 bitsInstead of keeping the 32- and 64- bit x86 bits separate, merge theminto common directories. This avoids a bunch of special cases in the build scriptswhile also enabling multilib builds.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-09  
Contributor(s):  
Keith Packard  
>libm/common: Use explicit 32-bit types in pow/log codeEnsure that computations generating float values are done with 32-bitvalues.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib/malloc: Use uintptr_t to hold pointer-sized valueReplace usage of 'unsigned long' with 'uintptr_t' to supporttargets where pointers and longs are different size.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Use uint32_t for ucs-4 valueswint_t might not be large enough for ucs-4, so we cannot reliably usethat to hold unicode code point values. Instead, explicitly useuint32_t.Signed-off-by: Keith Packard <keithp@keithp.com>  
>string: Use uint32_t for wcwidth/wcswidth internal ucs-4 valuesInstead of assuming that wint_t can hold ucs-4, use explicit uint32_tvariables.Signed-off-by: Keith Packard <keithp@keithp.com>  
>string: Support targets with 16-bit ints in optimized codeThe optimized string code is only used for 32-bit aligned data, butthe alignment checks were using 'long' instead of 'uintptr_t', causingproblems on targets with 16-bit pointers. Also, some computations needexplicit uint32_t casts to avoid truncation under the standard integerpromotion rules.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Fix up merge mistake with stdio cleanup callThe cleanup handler for regular stdio has moved to a thread localvariable and so needs to use the macro to access that instead ofusing a field in struct _reent.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib/strtodg: Add uint32_t casts when constructing float valuesMake sure all computations in strtodg.c are done with 32-bit intvalues.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdio: Get rid of struct _reent parameterThis is no longer used. This also means getting rid ofthe wrapped _ ... _r functions and exposing only the realstdio interface.Signed-off-by: Keith Packard <keithp@keithp.com>  
>reent: Eliminate reent.h and sys/reent.hClean up the last few files using these headers, thenremove them entirely.This removes a bunch of unused powerpc code as thatalso used reent, but gcc no longer supports the SPE optionneeded to use any of it.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master'  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-08  
Contributor(s):  
Keith Packard  
>mips: Clean up warnings in mips strncpyA couple of unused variables and several mystic FALLTHROUGHannotations used outside of a switch block.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math/tinystdio/stdlib: Avoid warnings when long double == doubleEven if long double and double are the same underlying machine type,gcc cares if you mix them without suitable casts. Disable somewarnings for pure function aliases and add some casts in other places.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.7.8' into debianVersion 1.7.8  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-06  
Contributor(s):  
Keith Packard  
>Add POSIX definitions to sys/features.h when building under ZephyrZephyr exposes a bunch of POSIX functions which are supposed to bedefined in C library headers. Add a section to features.h that setsthe flags necessary to make those headers include definitions for thenecessary functions.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-05  
Contributor(s):  
Keith Packard  
>newlib/stdio: Fix numerous type errors in el/ix level 4 stdio codeThis code had never been compiled with warnings enabled and had numerousinteger type mismatch warnings.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master'  
>Merge tag '1.7.7' into debianVersion 1.7.7  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-04  
Contributor(s):  
Keith Packard  
>tinystdio/string: Fix include directives to not depend on -I valueUse correct relative paths to local files to avoid depending on havingtinystdio and string in the compiler include path.Signed-off-by: Keith Packard <keithp@keithp.com>  
>m68k: Don't use unaligned access on m68010 memcpyIt's a stretch to call misaligned writes "OK" on 68010. It supportsthem in _software_, if and only if the operating system provides anaddress error handler that knows how to emulate theaccess. Newlib/picolibc does not provide this, and even if it did,vectoring to a software emulation routine for every singleword/longword access is WAY slower than just copying byte forbyte. The 68010 should probably be removed from this.Reported by: Alexis LockwoodSigned-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-03  
Contributor(s):  
Keith Packard  
>math: Introduce barriers in tgamma for clangClang doesn't understand that floating point operations haveside-effects affecting exception flags and happily allows extraevaluations even when there's a guard protecting them. Introduceclang_barrier_ macros that prevent optimization across theseboundaries, then use them in tgamma to prevent qNaN values fromsignaling FE_INVALID.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Make signgam follow posix and c99 specssigngam is required by posix and forbidden by c99, so use a hiddenvariable '__signgam' to hold the real value and create a weak aliasfor 'signgam' which is declared in math.h in posix mode but not in c99mode.Finally, we can't make signgam a thread local variable because c99allows applications to create global variables with that name, andbecause posix says it is defined as 'extern int signgam'. Applicationswanting thread safety are required by posix to use lgamma_r.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: add iszero and iseqsigiszero could use the existing fpclassify function while iseqsigrequired a bunch of new code, but none of that is machine specific.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Avoid clang "optimization" using conditional moves in powf and rintClang will compute a value and use a conditional move insteadbranching around the computation; if that computation could raise anexception, then we'll get the exception even if the condition issitting there to prevent it.Use the 'opt_barrier_' inlines which keep the compiler from doing thisoptimization. For now at least.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Provide helpers to set invalid exceptionThese helpers use arithmetic to set exceptions rather than poking the bitsdirectly. This makes sure that ABIs with hard floats and soft doubles getcorrect exceptions for each.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.7.6' into odebianVersion 1.7.6  
>Switch HAVE_ALIAS_* to _HAVE_ALIAS_*As these appear in picolibc.h, add an underscore to avoid collidingwith application definitions.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Add leading _ to 'HAVE' values included in picolibc.hThese values can control the contents of public headers so they'reincluded in picolibc.h, but they should use the reserved _ namespaceinstead of potentially conflicting with application names.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.7.5' into debianVersion 1.7.5  
>stdlib: Initialize getopt struct before checking early returnsThe initializer for getopt sets optind to zero, so if we don't run theinitialization sequence before checking for early returns, we'll getthe wrong answer.Signed-off-by: Keith Packard <keithp@keithp.com>  
>arm/risc-v: Move FAST_FMA defines to installed machine/math.h filesThis makes the inline fma functions available for applications usingpicolibc, not just for internal use. At the same time, add leading _to the HAVE_FAST_FMA macros so that they don't potentially conflictwith application symbols.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: mrand48, jrand48: fix 64-bit long sign extensionAdd casts to get the compiler to sign extend these results correctly.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Don't raise FE_OVERFLOW after FE_DIVBYZERO in tgammaA negative integer parameter to lgamma_r will raise FE_DIVBYZERO andreturn inf. The tgamma wrapper checks for inf and raises FE_OVERFLOWfor non-inf parameters, but we don't want to do that if we've alreadyraised FE_DIVBYZERO. Create a new __math_lgamma_r function whichreturns an indication that FE_DIVBYZERO has been raised and use thatto avoid also raising FE_OVERFLOW.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-02  
Contributor(s):  
Keith Packard  
>Merge remote-tracking branch 'newlib/master'  
>posix: Trim repeated / in dirnameConvert /a///b to /a instead of /a//This does not deal with the glibc hack that supports leading doubleslash, as was used in systems like UTek for remote file system access.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm: Add __math_err_check_uflowfNeeded for atan2 underflowSigned-off-by: Keith Packard <keithp@keithp.com>  
>math: Use 'fpowl' when compiling pow with FLT_EVAL_METHOD == 2In this mode, the intermediate values are all long double, so we needto use the long double version of abs. Most of the time, the fabsmacro figures this out all on its own, but on m68k it doesn't seem to.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2022-01  
Contributor(s):  
Keith Packard  
>Merge remote-tracking branch 'newlib/master'  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-12  
Contributor(s):  
Keith Packard  
>stdio: Leaveout eiisinf on non-x86 80-bit ldtoaWhen building legacy stdio's ldtoa function for m68k, the eiisinffunction gets included when it shouldn't as that function is onlyused on x86 hardware.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-11  
Contributor(s):  
Keith Packard  
>math: Use detected compiler data in math_config.hInstead of checking for __GNUC__ and assuming specific compilersupport, use meson detected values for various attributes and builtinfunctions. This also uses the always_inline attribute for functionsdefined in this header.Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Remove exception generation for soft float HWThis elides exception generating code sequences for hardware whichdoesn't have any floating point exception support, such as soft floatmachines.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Add clang no-builtin attribute support for memset/memmove/memcpy compilationThese functions all need to tell the compiler to not conver them intobuiltin calls as that generates an infinite recursion. For gcc, we usethe -fno-tree-loop-distribute-patterns flag, while clang uses theno-builtins function attribute (which does more useful things too).Signed-off-by: Keith Packard <keithp@keithp.com>  
>math: Add check_oflowf macroThis wraps the __math_check_oflowf function, skipping the call iferrno setting is disabled.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.7.4' into debianVersion 1.7.4  
>math: Add functions to manipulate inexact exceptionProvide a direct mechanism, rather than expecting functions that wantto return inexact to have custom code.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-10  
Contributor(s):  
Keith Packard  
Yasushi SHOJI  
>stdlib: Avoid warning for unused arg in __ldtoa'ldp' param is not used in normal paths.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc: Remove #ifdef check before #undefC Standards state that   It is ignored if the specified identifier is not currently defined   as a macro name.So we don't need to check with #ifdef before #undef.  It seems thatold compilers might error out, but we specify "c18" in themeson.build, anyway.newlib/libc/reent/getreent.c has the line "#undef __getreent" but it'snot added by the recent commit c2c593afa60b64bff, so this commitdoesn't change it.Signed-off-by: Yasushi SHOJI <yashi@spacecubics.com>  
>posix: Add basename, dirname and fnmatch back inThese were removed when removing POSIX file system stuff. They aren'tstrictly file system related, so we can include them.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm/common: Add __math_check_oflowfWill be used in libm/mathSigned-off-by: Keith Packard <keithp@keithp.com>  
>riscv: define functions with no args as (void)Make sure we don't have any K&R function definitions in the tree.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Remove unused 'j' in ldtoaSigned-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.7.3' into debianVersion 1.7.3  
>libm/test: Make sure string buffer target is large enoughThe string tests use '99' as the value for strncpy, and while they arecareful to avoid ever passing a string long enough to cause a bufferoverflow, the _FORTIFY_SOURCE code generates an exception if thespecified maximum length is larger than the destinationbuffer. Increase the destination buffer size to 100 to make theexisting tests not generate an exception.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Fix old-style function definitions.There were real K&R definitions all over the posix and searchcode. There were also a bunch of empty argument list functions thatdidn't use (void) in other places in the library.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc: Support _FORTIFY_SOURCE while building picolibcMake sure the ssp macro versions of various libc APIs are undefinedwhile defining the implementation. For strftime, we need to avoiddefining snprintf and strncmp as those will already be defined to bethe ssp wrappers. Instead, create unique names used for this function.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-09  
Contributor(s):  
Keith Packard  
Ayke van Laethem  
>Merge remote-tracking branch 'newlib/master'  
>stdlib: Fix typo in ldtoaThe code uses _alloc_dtoa_result, but it looks like it should be__alloc_dtoa_result.  
>newlib: Fixup for mergeFix a  couple of bugs introduced by the newlib merge: * Eliminate unused var warning in str2sig function   ('j' is used only on cygwin) * Eliminate unused var warning in gethex function   ('loc' is only used when locales are enabled)Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-08  
Contributor(s):  
Sebastian Meyer  
Keith Packard  
>Create FALLTHROUGH macro for use in switch statementsInstead of relying on special comments, use the new [[fallthrough]]annotation, or (failing that), the non-standard__attribute__((fallthrough) to indicate case clauses which shouldn'tend with a break statement.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Switch various constructs to C18 equivalentstypeof -> __typeofLONG_LONG_MIN/MAX -> LLONG_MIN/MAXBYTE_ORDER -> _BYTE_ORDERStruct initializers need .element = instead of element:Signed-off-by: Keith Packard <keithp@keithp.com>  
>Fix test code to eliminate warnings from -Wall -WextraSigned-off-by: Keith Packard <keithp@keithp.com>  
>Add #define _DEFAULT_SOURCE when building extended APIsAny API outside of C18 needs to set _DEFAULT_SOURCE so thatit will be declared by the headers.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Use size_t type to hold strlen result in setenvint may be too smallSigned-off-by: Keith Packard <keithp@keithp.com>  
>string: Switch length variables to 'size_t' in memmem and strstrThese functions were using int to hold memory object size information.Signed-off-by: Keith Packard <keithp@keithp.com>  
>wcwidth takes a wchar_t, not a wint_tThe standard says wchar_tSigned-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Fix casts in strtodSigned-off-by: Keith Packard <keithp@keithp.com>  
>Silence unused parameter warnings for APIs with defined interfacesLots of libc interfaces have interfaces which pass arguments whichpicolibc doesn't use; silence compiler warnings with (void) param;statements.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdio: Add casts to make signed/unsigned compares explicitstdio uses 'int' for various size values in the FILE struct, whichmakes gcc generate warnings when comparing against actual sizeinformation. Add explicit casts.Signed-off-by: Keith Packard <keithp@keithp.com>  
>_DEFAULT_SOURCE define is not be needed anymore  
>stdio: Fix references to _r functions outside of stdioStdio still had a few references to the _r versions of functions whichhave been removed. Switch these to the normal functions which are the only onesavailable now.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Replace remaining 'asm' with '__asm__'__asm__ is the C18 conforming name.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-07  
Contributor(s):  
Keith Packard  
>Merge tag '1.7' into debianVersion 1.7  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-06  
Contributor(s):  
Keith Packard  
>libm: Add inline functions for using ints for double/float operationsThese functions operate on floats and doubles with suitably-sizedinteger operations allowing simpler implementations than using theold multi-word code.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Use __asm__ instead of asm or __asmThis allows these statements to be compiled under --std=c18rules. While not important for the library (which needs to be compiledunder --std=gnu*), this is important for installed header files whichmay be compiled by applications using the more strict rules.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Make strtod/strtof set ERANGE consistently for underflow.The C standard says that errno may acquire the value ERANGE if theresult from strtod underflows. According to IEEE 754, underflow occurswhenever the value cannot be represented in normalized form.Newlib is inconsistent in this, setting errno to ERANGE only if thevalue underflows to zero, but not for denorm values, and never for hexformat floats.This patch attempts to consistently set errno to ERANGE for all'underflow' conditions, which is to say all values which are notexactly zero and which cannot be represented in normalized form.This matches glibc behavior, as well as the Linux, Mac OS X, OpenBSD,FreeBSD and SunOS strtod man pages.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge tag '1.6' into debianVersion 1.6  
>riscv: Don't define rounding modes or exception values for soft floatSigned-off-by: Keith Packard <keithp@keithp.com>  
>fenv: Move common definitions to include/fenv.hMove duplicated fenv definitions from architecture-specific files tocommon file.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-05  
Contributor(s):  
Keith Packard  
>newlib: Unify most C library locking into a single mutexThere's little reason to have a wealth of different locks in picolibc;none of the APIs covered are expected to be used in performancesensitive code. Reducing the number of mutexes shrinks the datasegment and simplifies the code.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-04  
Contributor(s):  
Keith Packard  
>Merge remote-tracking branch 'newlib/master'  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-12  
Contributor(s):  
Keith Packard  
>libc/stdlib: Work around compiler free optimizations in big mallocBig malloc also calls 'free' from memalign, and the compiler happilyoptimizes writes to that memory away, which breaks malloc. Fix this bycreating a special alias for free, __malloc_free, which keeps thecompiler at bay.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master' into main  
>libc: Fix mallocr merge errorExtra 'RCALL' was left in the code from newlib.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-11  
Contributor(s):  
Corinna Vinschen  
>malloc/nano-malloc: correctly check for out-of-bounds allocation reqsThe overflow check in mEMALIGn erroneously checks for INT_MAX,albeit the input parameter is size_t.  Fix this to check for__SIZE_MAX__ instead.  Also, it misses to check the req againstadding the alignment before calling mALLOc.While at it, add out-of-bounds checks to pvALLOc, nano_memalign,nano_valloc, and Cygwin's (unused) dlpvalloc.Signed-off-by: Corinna Vinschen <corinna@vinschen.de>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-10  
Contributor(s):  
Keith Packard  
>RISC-V: Use fmv.?.? instructions for float/int movesThis replaces the use of unions in asuint, asfloat, asuint64 andasdouble with the RISC-V instructions which perform these operationsdirectly in registers.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-09  
Contributor(s):  
Sebastian Meyer  
Keith Packard  
Jozef Lawrynowicz  
>Check for presence of __builtin_mul_overflow (#62)* Check for presence of __builtin_mul_overflowONLY if it's not there, use a software implementation.That software implementation assumes size_t to be compatible withCompCert (which does support neither statement expressions (=> nomacro magic) and no _Generic).In two of the three locations this is no problem at all, since they arehardcoded to operate on size_t. But the third location only defaults tosize_t - but that can be overwritten by some define.However, I don't think that's a real problem: Previously compilerswithout __builtin_mul_overflow were not supported at all, now they'resupported as long as the user sticks to a default setting.An appropriate `#error` has been added.* Check for 0, and better phrasing in #errorThe mul_overflow could divide by zero, this is now prevented.Also use the more precise phrasing suggested by Keith.There is a possible optimization for mul_overflow:As Hacker's Delight points out, the overflow check can be improved.Assume a 32 bit type (and nlz = number of leading zeroes)> nlz(a) + nlz(b) >= 32: Does not overflow> nlz(a) + nlz(b) == 31: Needs expensive check> nlz(a) + nlz(b) <= 30: Will overflowHowever, counting the number of leading zero now needs another builtin,which would need another check, which would need a (slow) fall-backimplementation again, so I'm not sure if I want to open this Pandora'sBox right now.A less precise (but more universal) variant can be achieved byapproximating the first term with> (a + b < a || a + b < b): Does not overflow> Anything else: checkHowever, it seems this might get optimized as "false" since the compilermight assume that there was no overflow (at least from what I read).So for now I'll stick with KISS - after all, all previously usedcompilers have __builtin_mul_overflow anyway.* Improve the test for __builtin_mul_overflowThe test now tries to link the program. This avoids false positivedetection in case the compiler handles the (unknown) builtin like aimplicit declaration.  
>libc: Compatiblity fixes for CompCert * Set `_DEFAULT_SOURCE` in features.h * Protect size_t typedef with _SIZE_T as well as _SIZE_T_DECLARED  
>Fix warnings when building for msp430-elfThe MSP430 target supports both 16-bit and 20-bit size_t and intptr_t.Some implicit casts in Newlib expect these types to be"long", (a 32-bit type on MSP430) which causes warnings duringcompilation such as:  "cast from pointer to integer of different size"  
>Merge remote-tracking branch 'newlib/master' into main  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-08  
Contributor(s):  
Keith Packard  
Angus Gratton  
>libm: Control errno support with _IEEE_LIBM configuration parameterThis removes the run-time configuration of errno support present inportions of the math library and unifies all of the compile-time errnoconfiguration under a single parameter so that the whole libraryis consistent.The run-time support provided by _LIB_VERSION is no longer present inthe public API, although it is still used internally to disable errnosetting in some functions. Now that it is a constant, the compiler shouldremove that code when errno is not supported.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm: ARM without HW double does not have fast FMA32-bit ARM processors with HW float (but not HW double) may define__ARM_FEATURE_FMA, but that only means they have fast FMA for 32-bitfloats.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm/machine/riscv: Add custom fma/sqrt functions when supportedCheck for HW FMA and SQRT support and use those instructions in placeof software implementations.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm: Split __OBSOLETE_MATH into float/double definesCreate two new defines, __OBSOLETE_MATH_FLOAT and__OBSOLETE_MATH_DOUBLE which independently control whether the newfloat and new double implementations of various functions are used ornot. This allows us to always use the new double implementations bydefault, but select the old float implementations for processorswithout double hardware support.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm/common: Set WANT_ERRNO based on _IEEE_LIBM value_IEEE_LIBM is the configuration value which controls whether theoriginal libm functions modify errno. Use that in the new math code aswell so that the resulting library is internally consistent.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm/risc-v: Use __riscv_flen >= 64 instead of == 64Future hardware may well support 128 bit floats ...Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc/stdlib: Use __builtin_mul_overflow for reallocarray and callocThis built-in function (available in both gcc and clang) is moreefficient and generates shorter code than open-coding the test.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Avoid implicit floating point conversions [v2]These were found with clang -Wdouble-promotion and show places wherefloating point values were being implicitly converted betweenrepresentations. These conversions can result in unexpected use ofdouble precision arithmetic. Those which are intentional all have anexplicit cast added.Signed-off-by: Keith Packard <keithp@keithp.com>v2:	Use clang built-in classification functions.	Declare constants as 'float' instead of double.  
>Merge tag '1.4.6' into debianVersion 1.4.6  
>libm: Detect fast fmaf supportAnything with fast FMA is assumed to have fast FMAF, along with32-bit arms that advertise 32-bit FP support and __ARM_FEATURE_FMASigned-off-by: Keith Packard <keithp@keithp.com>  
>libc/stdlib: Clean up clang warningsCheck for clang before using GCC-specific pragmaEliminate hand-written strcpy in setenv.Signed-off-by: Keith Packard <keithp@keithp.com>  
>riscv: Use optimized memcpy,memove,memset,strcmp,strcpy,strlen with -OsIn upstream code, building libc with -Os will disable theoptimized versions of these (falling back to byte-oriented versions).Removing -Os and setting -O2 increases the total libc size by a largeamount (+22KB for a ROM build), but just enabling these is only +1KB or so.This also brings memcpy() behaviour in sync with our Xtensa newlib builds, whichuse the optimized versions of these functions even with -Os  
>libc/riscv: Remove conditions around fenv value definitionsThe definitions of the various exception and rounding don't need tochange based on whether the hardware has support.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-07  
Contributor(s):  
PkmX via Newlib  
Keith Packard  
>Merge remote-tracking branch 'newlib/master' into main  
>riscv: fix integer wraparound in memcpyThis patch fixes a bug in RISC-V's memcpy implementation where aninteger wraparound occurs when src + size < 8 * sizeof(long), causingthe word-sized copy loop to be incorrectly entered.Signed-off-by: Chih-Mao Chen <cmchen@andestech.com>  
>libc/tinystdio: Rework how smaller printf functions workInstead of using the C preprocessor to rename all of the printf andscanf functions, use the linker to re-direct the lowest levelfunctions (vfprintf and vfscanf) to the custom versions. This requiresplacing the definition of the C preprocessor names on the compile andlink lines where the .specs file can see them, but avoids linkingmultiple printf/scanf variants into applications.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libm: Eliminate extra _fe_dfl_env variable in several fenv versionsMake the exported symbol be the struct instead of a using a separatepointer value. Fix the FE_DFL_ENV macro to return a const fenv_t *pointing to that.This fixes the stub version, riscv and x86_64Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc/stdio: Switch _malloc_r/_free_r to malloc/freepicolibc doesn't provide _malloc_r or _free_rSigned-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-06  
Contributor(s):  
Denis Feklushkin  
Ivan Grokhotkov  
Keith Packard  
>Merge remote-tracking branch 'newlib/master' into main  
>Remove offensive wordingSigned-off-by: Keith Packard <keithp@keithp.com>  
>__assert args order as in glibc  
>enable idf-specific pthread features for risc-v as well  
>Merge tag '1.4.3' into debianVersion 1.4.3  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-05  
Contributor(s):  
Keith Packard  
>stdlib: Only build strtold et al on 80-bit long double systemsThe stdlib implementation of strtold and friends only supports 80-bitfloating point format, don't build it for other systems (likeRISC-V). There is a strtold which supports both 80- and 128- bit longdouble as a part of tinystdio, although that version doesn't supportlocale-specific parsing.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdio/stdlib: _dtoa_r → __dtoa, _ldtoa_r → __ldtoa_ldtoa_r didn't have a definition in any header file, which led to itbeing called with the wrong parameters. Now both __dtoa and __ldtoaare defined in stdlib.h, and called with the right parameters, whichreally helps make them work right.This also means we can delete dtoastub.c as the internal __dtoaprovides the same API.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc/stdlib: Use malloc directly for mprec functionsInstead of wrapping malloc with a custom allocator, just call itdirectly. This avoids holding memory in the mprec code.Signed-off-by: Keith Packard <keithp@keithp.com>  
>stdlib: Build strtold, wcstold with long double. Always build strtod_lstrtold and wcstold had a bunch of lingering bugs around 'struct reent' andextra underscores.strtod_l is needed on tinystdio systems as tinystdio only provides strtod.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc/stdlib: Remove debug printf from _strtorx_lThis snuck in at some point.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc/stdlib: Handle malloc failures in mprec routinesConverting strings and floats requires long precision arthmetic, whichuses malloc to make space for temporary values. If malloc fails, weneed to return errors back to the application, rather than calling an'abort' function which the application cannot recover from.This required a number of changes to check for NULL values through theentire stack of code that depends on the mprec code.Signed-off-by: Keith Packard <keithp@keithp.com>  
>libc/tinystdio: Create tinystdio versions of *cvt*This replaces the rest of the multi-precision based float/stringconversion functions from newlib with imprecise versions that usefloating point types.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-04  
Contributor(s):  
Keith Packard  
張辰謙  
Ayke van Laethem  
>newlib/libc/machine/riscv: Put asm functions in separate text sectionsJust makes these match the rest of the library when compiling with-ffunction-sections.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Fix unbalanced diagnostics pragmaClang 9 is complaining about this and the code appears to be wrong.  
>Correct the typo in the DESCRIPTIONOld: copies not more than <[length]> characters from the the stringNew: copies not more than <[length]> characters from the string  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-03  
Contributor(s):  
Keith Packard  
>Merge remote-tracking branch 'newlib/master'Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-02  
Contributor(s):  
Keith Packard  
TheDennenning  
>Add missing copyrights to newlib/libc/searchSigned-off-by: Keith Packard <keithp@keithp.com>  
>Suppressing Compiler Warnings in string test casesAdd #pragma statements needed to supress compiler warnings intest cases which are expected due to what the test is doing.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Add missing copyrights in newlib/libc/stdio64Signed-off-by: Keith Packard <keithp@keithp.com>  
>Typo in license terms for newlib/libm/common/log2.cThe closing quotes were in the wrong placeSigned-off-by: Keith Packard <keithp@keithp.com>  
>Add missing copyrights in newlib/libc/stdlibSigned-off-by: Keith Packard <keithp@keithp.com>  
>Add missing copyrights to newlib/libc/machineSigned-off-by: Keith Packard <keithp@keithp.com>  
>Typo in license for newlib/libc/stdio/flags.cFix spelling:	MERCHANT I BILITY -> MERCHANT A BILITYSigned-off-by: Keith Packard <keithp@keithp.com>  
>Add missing copyrights in newlib/libm/mathSigned-off-by: Keith Packard <keithp@keithp.com>  
>Add Copyright where missing to newlib/libc/include filesMostly derived from files added in the same commit. Some copyrightsderived from commit messages.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Add missing copyrights in newlib/libc/stringSigned-off-by: Keith Packard <keithp@keithp.com>  
>Add copyright and license to newlib/libm/test/*.cThe Makefile.in in this directory has a Copyright 1994 Cygnus andan old-style BSD license. I've copied that into all of the .c files.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-01  
Contributor(s):  
Keith Packard  
>riscv: Add 'break' statements to fpsetround switchThis makes the fpsetround function actually do something rather thanjust return -1 due to the default 'fall-through' behavior of the switchstatement.Signed-off-by: Keith Packard <keithp@keithp.com>  
>riscv: Use current pseudo-instructions to access the FCSR registerUse fscsr and frcsr to store and read the FCSR register instead offssr and frsr.Signed-off-by: Keith Packard <keithp@keithp.com>  
>riscv: Addfpgetroundtoi and fpsetroundtoi stubsI've found no description of what these functions are supposed to do,so I'm not even going to try and implement them.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master'  
>riscv: Map between ieeefp.h exception bits and RISC-V FCSR bitsIf we had architecture-specific exception bits, we could just set themto match the processor, but instead ieeefp.h is shared by all targetsso we need to map between the public values and the register contents.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-12  
Contributor(s):  
Keith Packard  
>Use 'private' names for int/float versions of printf/scanfInstead of iprintf/printff, use __i_printf and __f_printf. This shouldmake it easier to spot trouble, as well as avoid accidental collisionswith application names.Also hack up stdio.h so that the old stdio can still be used with theinternal users of the new names.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master'  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-11  
Contributor(s):  
Keith Packard  
>Place all configuration data in 'picolibc.h'Create a newlib.h that includes picolibc.hRemove _newlib_version.hFix newlib/libc/include/sys/features.h to use picolibc.hSigned-off-by: Keith Packard <keithp@keithp.com>  
>Append 'ul' to integer constants in newlib/libm/common/sf_pow.cThis makes it compile for MSP430 without warnings.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master'  
>Patch for malloc typosThe previous patch found typos in nano-malloc. Those same typosoccur in the larger malloc implementation.Fix #define of rEALLOc to be reallocRename malloc-mallinco.c to malloc-mallinfo.c and fix the #define inside of thatSigned-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-10  
Contributor(s):  
Keith Packard  
>Merge remote-tracking branch 'newlib/master' into masterBring in current newlib bits, leaving out all of the files we've deletedSigned-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-09  
Contributor(s):  
Keith Packard  
>meson: Add HAVE_INIT_FINI configurationThis adds support for _init and _fini calls from crt0 and makes themoptional.Signed-off-by: Keith Packard <keithp@keithp.com>  
>remove reent.h from string funcs  
>Actually build iconv bits, fixing the TLS stuffThe iconv bits weren't actually getting built, and so they hadn't beenconverted to use TLSSigned-off-by: Keith Packard <keithp@keithp.com>  
>Fix compiler warnings about missing () in riscv/ieeefp.cThis code relied upon operator precedence; the compiler would preferto see parens.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Clean up some on_exit codeon_exit_args.c is no longer needed now that this code uses TLS.Signed-off-by: Keith Packard <keithp@keithp.com>  
>remove reent.h from more string funcs  
>Make math tests build with glibc as well as picolibcThis lets us compare glibc and picolibc results.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Remove #include <reent.h> from stdio source filesstdio.h already includes this fileSigned-off-by: Keith Packard <keithp@keithp.com>  
>libc/string: Wrap size-optimized string operation loops in ()This avoids a compiler warning.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Only call _GLOBAL_REENT->__cleanup on !newlib_tinystdioSigned-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-08  
Contributor(s):  
Keith Packard  
>Include errno.h from math_config.hThis isn't getting included through other paths anymoreSigned-off-by: Keith Packard <keithp@keithp.com>  
>Use non-reent external funcs for stdioFor all low-level newlib and syscall functions, use the versionswithout reent as those are the only available versions now.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Use TLS for string functionsRemove _r variants.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Use TLS for environment variable functionsSigned-off-by: Keith Packard <keithp@keithp.com>  
>Use TLS for random number generatorsSigned-off-by: Keith Packard <keithp@keithp.com>  
>Declare two_way_short_needle as 'inline'This avoids a warning when including this function in files whichdon't use it.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Use TLS for locale/wchar/multibyte functionsSigned-off-by: Keith Packard <keithp@keithp.com>  
>Merge remote-tracking branch 'newlib/master'  
>libm/common: Split math_err.c and math_errf.c into per-function filesThis avoids bringing in additional code when using error functionswithout --gc-sections.Two functions from each file are now global and have been renamed to start with __math:	with_errno → __math_with_errno	with_errnof → __math_with_errnof	xflow → __math_xflow	xflowf → __math_xflowfThere should be no functional changes to the resulting code.Signed-off-by: Keith Packard <keithp@keithp.com>  
>Remove '#include <reent.h>' where no longer neededThese files don't provide any re-entrant APIsSigned-off-by: Keith Packard <keithp@keithp.com>  
>Remove 'reent' from mallocr.cmallocr uses locks instead of per-thread storage, so its use ofreent was strictly for pass-through to malloc_lockSigned-off-by: Keith Packard <keithp@keithp.com>  
>Use TLS for strsignalSigned-off-by: Keith Packard <keithp@keithp.com>  
>Use TLS for multi-precision functionsThis also means using malloc directly instead of through _malloc_r.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-07  
Contributor(s):  
Keith Packard  
>Merge remote-tracking branch 'newlib/master'Remove newlib/libm/common/s_matherr.cRename newlib/libc/include/xlocale.h -> newlib/libc/include/sys/_locale.h  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-04  
Contributor(s):  
Anton Maklakov  
>libc: Remove ESP-IDF specific *_VISIBLE macros from xtensa config because we support a generic libc  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-03  
Contributor(s):  
Dmitry Plotnikov  
>libc: added _POSIX_TIMERS, _POSIX_TIMEOUTS and _POSIX_MONOTONIC_CLOCK for xtensa  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-11  
Contributor(s):  
Keith Packard  
>Merge commit '2ab57ad59bc35dafffa69cd4da5e228971de069f'  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-09  
Contributor(s):  
Keith Packard  
>Use __errno_r(ptr) instead of ptr->_errnoThis uses the __errno_r macro to access the errno value in the reentstruct instead of directly accessing it.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-08  
Contributor(s):  
Keith Packard  
>Use nanf("") instead of nanf(NULL)Newer GCC versions require a non-NULL argument to this function forsome reason.Signed-off-by: Keith Packard <keithp@keithp.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-06  
Contributor(s):  
Remy Bohmer  
Alexey Gerenkov  
>xtensa: enable pthreads support  
>libc: xtensa: enable pthreads support  
>ct-ng bundled patch: 0000-fix-unaligned-access-memcpy-m68k.patchThe m68k mcpu processor does not like unaligned accessDisable at least mcpu32, m68010 and m68020. These processors certainlydo not like unaligned accesses.Signed-off-by: Remy Bohmer <linux@bohmer.net>[yann.morin.1998@anciens.enib.fr: update for 1.19.0 from 1.18.0]Signed-off-by: "Yann E. MORIN" <yann.morin.1998@anciens.enib.fr>[austinpmorton@gmail.com: update for 1.20.0 from 1.19.0]Signed-off-by: Austin Morton <austinpmorton@gmail.com>  
>Make GNU and some misc funcs visible  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-05  
Contributor(s):  
Ivan Grokhotkov  
>libc: Make some data const  
>Make some data const  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-07  
Contributor(s):  
Angus Gratton  
>xtensa: cleanupsSigned-off-by: Max Filippov <jcmvbkbc@gmail.com>  
>xtensa: Add portconfigure.host: flags for xtensa* REENTRANT_SYSCALLS_PROVIDED is a host property, not CPU arch  dependent. Move it to host related part.* HAVE_RENAME_R and HAVE_GETTIMEOFDAY do not exist in newlib anymore* ABORT_PROVIDED is likely not needed, and it is easier to keep newlib  abort implementation for bare metal builds, and override it as  needed.* GETREENT_PROVIDED allows us to drop getreent disabling patch.* HAVE_BLKSIZE has been requested a while agolibm: xtensa: enable sqrt, __ieee754_remainder[f], __ieee754_fmod[f](no them in xtensa libgcc)Signed-off-by: Max Filippov <jcmvbkbc@gmail.com>Signed-off-by: Angus Gratton <gus@projectgus.com>  
>xtensa: add portSigned-off-by: Max Filippov <jcmvbkbc@gmail.com>Signed-off-by: Angus Gratton <gus@projectgus.com>  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 

