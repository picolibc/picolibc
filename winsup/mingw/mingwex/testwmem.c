#include <memory.h>
#include <wchar.h>
#include <stdio.h>

wchar_t fmt1[] =   L"         1         2         3         4         5";
wchar_t fmt2[] =   L"12345678901234567890123456789012345678901234567890";

void test_wmemchr( void )
{
   wchar_t* dest;
   wint_t result;
   wint_t ch = L'r';
   wchar_t str[] =   L"lazy";
   wchar_t string1[60] = L"The quick brown dog jumps over the lazy fox";

   wprintf( L"Wmemchr\n" );
   wprintf( L"String to be searched:\n\t\t%s\n", string1 );
   wprintf( L"\t\t%s\n\t\t%s\n\n", fmt1, fmt2 );

   wprintf( L"Search char:\t%c\n", ch );
   dest = wmemchr( string1, ch, sizeof( string1 ) );
   result = dest - string1 + 1;
   if( dest != NULL )
      wprintf( L"Result:\t\t%c found at position %d\n\n", ch, result );
   else
      wprintf( L"Result:\t\t%c not found\n\n" );
return;
}
void test_wmemset( void )
{/*                               1         2 
                        0123456789012345678901234567890 */
   wchar_t buffer[] = L"This is a test of the wmemset function";
   wprintf( L"Before: %s\n", buffer );
   wmemset( buffer+22, L'*', 7 );
   wprintf( L"After:  %s\n\n", buffer );
return;
}

void test_wmemmove( void )
{
    wchar_t string1[60] = L"The quick brown dog jumps over the lazy fox";
    wchar_t string2[60] = L"The quick brown fox jumps over the lazy dog";

    wprintf( L"Wmemcpy without overlap\n" );
    wprintf( L"Source:\t\t%s\n", string1 + 40 );
    wprintf( L"Destination:\t%s\n", string1 + 16 );
    wmemcpy( string1 + 16, string1 + 40, 3 );
    wprintf( L"Result:\t\t%s\n", string1 );
    wprintf( L"Length:\t\t%d characters\n\n", wcslen( string1 ) );
    wmemcpy( string1 + 16, string2 + 40, 3 );

   wprintf( L"Wmemmove with overlap\n" );
   wprintf( L"Source:\t\t%s\n", string2 + 4 );
   wprintf( L"Destination:\t%s\n", string2 + 10 );
   wmemmove( string2 + 10, string2 + 4, 40 );
   wprintf( L"Result:\t\t%s\n", string2 );
   wprintf( L"Length:\t\t%d characters\n\n", wcslen( string2 ) );

   wprintf( L"Wmemcpy with overlap\n" );
   wprintf( L"Source:\t\t%s\n", string1 + 4 );
   wprintf( L"Destination:\t%s\n", string1 + 10 );
   wmemcpy( string1 + 10, string1 + 4, 40 );
   wprintf( L"Result:\t\t%s\n", string1 );
   wprintf( L"Length:\t\t%d characters\n\n", wcslen( string1 ) );
}


void test_wmemcmp( void )
{
   wchar_t first[]  = L"12345678901234567890";
   wchar_t second[] = L"12345678901234567891";
   wint_t result;
   wprintf(L"Wmemcmp\n"); 
   wprintf( L"Compare '%.19s' to '%.19s':\n", first, second );
   result = wmemcmp( first, second, 19 );
   if( result < 0 )
      wprintf( L"First is less than second.\n" );
   else if( result == 0 )
      wprintf( L"First is equal to second.\n" );
   else if( result > 0 )
      wprintf( L"First is greater than second.\n" );
   wprintf( L"\nCompare '%.20s' to '%.20s':\n", first, second );
   result = wmemcmp( first, second, 20 );
   if( result < 0 )
      wprintf( L"First is less than second.\n\n" );
   else if( result == 0 )
      wprintf( L"First is equal to second.\n\n" );
   else if( result > 0 )
      wprintf( L"First is greater than second.\n\n" );
}



int main(){
test_wmemset();
test_wmemmove();
test_wmemchr();
test_wmemcmp();
return 0;
}




