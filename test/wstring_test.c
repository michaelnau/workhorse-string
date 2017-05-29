#define _GNU_SOURCE
#include "wstring_sugar.h"
#define TEST_IMPLEMENTATION
#include "Testing.h"
#include <stdlib.h>

WStringNamespace s = wstringNamespace;

#define autoChar __attribute__(( cleanup( safeFree ))) char

static void
safeFree( char** pointer ) {
	if ( pointer ) {
        free( *pointer );
        *pointer = NULL;
	}
}

//---------------------------------------------------------------------------------

void Test_wstring_newDup()
{
	autoWString *string1 = wstring_new( "", 0 );
	assert_strequal( string1->cstring, "" );

	autoWString *string2 = wstring_dup( "test" );
	assert_strequal( string2->cstring, "test" );

	autoWString *string3 = wstring_dup( "Weiße Möhren" );
	assert_strequal( string3->cstring, "Weiße Möhren" );

	autoWString* string4 = wstring_dup( "" );
	assert_strequal( string4->cstring, "" );

	autoWString* string5 = wstring_dup( "test" );
	assert_strequal( string5->cstring, "test" );

	autoWString* string6 = wstring_dup( "Weiße Möhren" );
	assert_strequal( string6->cstring, "Weiße Möhren" );
}
void Test_wstring_printf()
{
	autoWString *string1 = wstring_printf( "%s", "test" );
	assert_strequal( string1->cstring, "test" );

	autoWString *string2 = wstring_printf( "test %s", "test" );
	assert_strequal( string2->cstring, "test test" );

	autoWString *string3 = wstring_printf( "Weiße %s", "Möhren" );
	assert_strequal( string3->cstring, "Weiße Möhren" );

	autoWString *string4 = wstring_printf( "1 + %d = 2", 1 );
	assert_strequal( string4->cstring, "1 + 1 = 2" );

	autoWString *string5 = wstring_printf( "My name is %s, %s %s.", "Bond", "James", "Bond" );
	assert_strequal( string5->cstring, "My name is Bond, James Bond." );
}
void Test_wstring_clone()
{
	autoWString *string1a = wstring_new( "", 0 );
	autoWString *string1b = wstring_clone( string1a );
	assert_strequal( string1b->cstring, "" );

	autoWString *string2a = wstring_dup( "test" );
	autoWString *string2b = wstring_clone( string2a );
	assert_strequal( string2b->cstring, "test" );

	autoWString *string3a = wstring_dup( "Weiße Möhren" );
	autoWString *string3b = wstring_clone( string3a );
	assert_strequal( string3b->cstring, "Weiße Möhren" );
}
void Test_wstring_stealCstring()
{
	autoWString *string1 = wstring_new( "", 0 );
	autoChar* cstring1a = wstring_cstring( string1 );
	autoChar* cstring1b = wstring_steal( &string1 );
	assert_strequal( cstring1a, "" );
	assert_strequal( cstring1b, "" );

	autoWString* string2 = wstring_dup( "test" );
	autoChar* cstring2a = wstring_cstring( string2 );
	autoChar* cstring2b = wstring_steal( &string2 );
	assert_strequal( cstring2a, "test" );
	assert_strequal( cstring2b, "test" );

	autoWString* string3 = wstring_dup( "Weiße Möhren" );
	autoChar* cstring3a = wstring_cstring( string3 );
	autoChar* cstring3b = wstring_steal( &string3 );
	assert_strequal( cstring3a, "Weiße Möhren" );
	assert_strequal( cstring3b, "Weiße Möhren" );
}

//---------------------------------------------------------------------------------

void Test_wstring_compareCompareCaseEquals()
{
	autoWString* string1a = wstring_dup("");
	autoWString* string1b = wstring_dup("");
	assert_equal( wstring_compare( string1a, string1b ), 0 );
	assert_equal( wstring_compareCase( string1a, string1b ), 0 );
	assert_true( wstring_equals( string1a, string1b ));

	autoWString* string2a = wstring_dup("A");
	autoWString* string2b = wstring_dup("A");
	assert_equal( wstring_compare( string2a, string2b ), 0 );
	assert_equal( wstring_compareCase( string2a, string2b ), 0 );
	assert_true( wstring_equals( string2a, string2b ));

	autoWString* string3a = wstring_dup("A");
	autoWString* string3b = wstring_dup("a");
	assert_less( wstring_compare( string3a, string3b ), 0 );
	assert_equal( wstring_compareCase( string3a, string3b ), 0 );
	assert_false( wstring_equals( string3a, string3b ));

	autoWString* string4a = wstring_dup("Handsome");
	autoWString* string4b = wstring_dup("handfull");
	assert_less( wstring_compare( string4a, string4b ), 0 );
	assert_greater( wstring_compareCase( string4a, string4b ), 0 );
	assert_false( wstring_equals( string4a, string4b ));
}
void
Test_wstring_similarity()
{
	autoWString* string1 = s.new( "", 0 );
	autoWString* string2 = s.new( "", 0 );
	assert_equal( wstring_similarity( string1, string2 ), 0 );

	s.appendc( string2, "T" );
	assert_equal( wstring_similarity( string1, string2 ), 1 );

	s.appendc( string1, "T" );
	assert_equal( wstring_similarity( string1, string2 ), 0 );

	s.appendc( string2, "est" );
	assert_equal( wstring_similarity( string1, string2 ), 3 );

	s.appendc( string1, "est" );
	assert_equal( wstring_similarity( string1, string2 ), 0 );

	s.assign( &string1, s.dup( "Tess" ));
	assert_equal( wstring_similarity( string1, string2 ), 1 );

	s.assign( &string2, s.dup( "Tent" ));
	assert_equal( wstring_similarity( string1, string2 ), 2 );

	s.assign( &string1, s.dup( "kitten" ));					//From Wikipedia article
	s.assign( &string2, s.dup( "sitting" ));
	assert_equal( wstring_similarity( string1, string2 ), 3 );
}

//---------------------------------------------------------------------------------

void
Test_wstring_append()
{
	autoWString *string1 = wstring_dup( "" );
	autoWString *string2 = wstring_dup( "" );
	assert_strequal( wstring_append( string1, string2 )->cstring, "" );

	autoWString *string3 = wstring_dup( "" );
	autoWString *string4 = wstring_dup( "test" );
	assert_strequal( wstring_append( string3, string4 )->cstring, "test" );

	autoWString *string5 = wstring_dup( "test" );
	autoWString *string6 = wstring_dup( "" );
	assert_strequal( wstring_append( string5, string6 )->cstring, "test" );

	autoWString *string7 = wstring_dup( "test" );
	autoWString *string8 = wstring_dup( "test" );
	assert_strequal( wstring_append( string7, string8 )->cstring, "testtest" );

	autoWString *string9 = wstring_dup( "Weiße " );
	autoWString *string10 = wstring_dup( "Möhren" );
	assert_strequal( wstring_append( string9, string10 )->cstring, "Weiße Möhren" );
}
void
Test_wstring_appendf()
{
	autoWString* string = s.dup("");
	s.appendf( string, "" );
	assert_strequal( string->cstring, "" );

	s.appendf( string, "Test" );
	assert_strequal( string->cstring, "Test" );

	s.appendf( string, "%u", 1234 );
	assert_strequal( string->cstring, "Test1234" );

	s.appendf( string, "%s", " Test" );
	assert_strequal( string->cstring, "Test1234 Test" );

	s.appendf( string, "%s%u", " Test", 1234 );
	assert_strequal( string->cstring, "Test1234 Test Test1234" );
}
void
Test_wstring_prepend()
{
	autoWString *string1 = wstring_dup( "" );
	autoWString *string2 = wstring_dup( "" );
	assert_strequal( wstring_prepend( string1, string2 )->cstring, "" );

	autoWString *string3 = wstring_dup( "" );
	autoWString *string4 = wstring_dup( "test" );
	assert_strequal( wstring_prepend( string3, string4 )->cstring, "test" );

	autoWString *string5 = wstring_dup( "test" );
	autoWString *string6 = wstring_dup( "" );
	assert_strequal( wstring_prepend( string5, string6 )->cstring, "test" );

	autoWString *string7 = wstring_dup( "test" );
	autoWString *string8 = wstring_dup( "test" );
	assert_strequal( wstring_prepend( string7, string8 )->cstring, "testtest" );

	autoWString *string9 = wstring_dup( "Möhren" );
	autoWString *string10 = wstring_dup( "Weiße " );
	assert_strequal( wstring_prepend( string9, string10 )->cstring, "Weiße Möhren" );
}

//---------------------------------------------------------------------------------

void Test_wstring_ltrim()
{
	autoWString *string1 = wstring_dup( "" );
	assert_strequal( wstring_ltrim( string1, " " )->cstring, "" );

	autoWString *string2 = wstring_dup( "test" );
	assert_strequal( wstring_ltrim( string2, " " )->cstring, "test" );

	autoWString *string3 = wstring_dup( " test" );
	assert_strequal( wstring_ltrim( string3, " " )->cstring, "test" );

	autoWString *string4 = wstring_dup( "\n\rtest" );
	assert_strequal( wstring_ltrim( string4, "\r\n" )->cstring, "test" );

	autoWString *string5 = wstring_dup( "\ttest" );
	assert_strequal( wstring_ltrim( string5, "\t" )->cstring, "test" );

	autoWString *string6 = wstring_dup( "\t   \r \n \t\n\t  test" );
	assert_strequal( wstring_ltrim( string6, " \r\n\t" )->cstring, "test" );

	autoWString *string7 = wstring_dup( "\n  t test" );
	assert_strequal( wstring_ltrim( string7, " \n" )->cstring, "t test" );

	autoWString *string8 = wstring_dup( "\n  test\n\n\n" );
	assert_strequal( wstring_ltrim( string8, " \n" )->cstring, "test\n\n\n" );
}
void Test_wstring_rtrim()
{
	autoWString *string1 = wstring_dup( "" );
	assert_strequal( wstring_rtrim( string1, " " )->cstring, "" );

	autoWString *string2 = wstring_dup( "test" );
	assert_strequal( wstring_rtrim( string2, " " )->cstring, "test" );

	autoWString *string3 = wstring_dup( "test " );
	assert_strequal( wstring_rtrim( string3, " " )->cstring, "test" );

	autoWString *string4 = wstring_dup( "test\r\n" );
	assert_strequal( wstring_rtrim( string4, "\r\n" )->cstring, "test" );

	autoWString *string5 = wstring_dup( "test\t" );
	assert_strequal( wstring_rtrim( string5, "\t" )->cstring, "test" );

	autoWString *string6 = wstring_dup( "test\t   \r \n \t\n\t  " );
	assert_strequal( wstring_rtrim( string6, " \r\n\t" )->cstring, "test" );

	autoWString *string7 = wstring_dup( "test\n  t  ");
	assert_strequal( wstring_rtrim( string7, " " )->cstring, "test\n  t" );

	autoWString *string8 = wstring_dup( "\n  test\n\n\n" );
	assert_strequal( wstring_rtrim( string8, "\n" )->cstring, "\n  test" );

}
void Test_wstring_trim()
{
	autoWString *string1 = wstring_dup( "" );
	assert_strequal( wstring_trim( string1, " " )->cstring, "" );

	autoWString *string2 = wstring_dup( "test" );
	assert_strequal( wstring_trim( string2, " " )->cstring, "test" );

	autoWString *string3 = wstring_dup( "\n\r   \r\ntest" );
	assert_strequal( wstring_trim( string3, " \r\n" )->cstring, "test" );

	autoWString *string4 = wstring_dup( "test\n\r   \r\n" );
	assert_strequal( wstring_trim( string4, " \r\n" )->cstring, "test" );

	autoWString *string5 = wstring_dup( "\t\t\t\n   \ntest\n   \n" );
	assert_strequal( wstring_trim( string5, " \n\t" )->cstring, "test" );

	autoWString *string6 = wstring_dup( "[   test   ]" );
	assert_strequal( wstring_trim( string6, "[] " )->cstring, "test" );
}

//---------------------------------------------------------------------------------

void Test_wstring_squeeze()
{
	autoWString *string1 = wstring_dup( "" );
	assert_strequal( wstring_squeeze( string1 )->cstring, "" );

	autoWString *string2 = wstring_dup( " " );
	assert_strequal( wstring_squeeze( string2 )->cstring, " " );

	autoWString *string3 = wstring_dup( "a" );
	assert_strequal( wstring_squeeze( string3 )->cstring, "a" );

	autoWString *string4 = wstring_dup( " a" );
	assert_strequal( wstring_squeeze( string4 )->cstring, " a" );

	autoWString *string5 = wstring_dup( "  a" );
	assert_strequal( wstring_squeeze( string5 )->cstring, " a" );

	autoWString *string6 = wstring_dup( "     a   b\t\t\t\t c\t\t  d   " );
	assert_strequal( wstring_squeeze( string6 )->cstring, " a b\t c\t d " );

	autoWString *string7 = wstring_dup( "         Howard    Carpendale     " );
	assert_strequal( wstring_squeeze( string7 )->cstring, " Howard Carpendale " );

	autoWString *string8 = wstring_dup( "         heiße   Sägezähne         " );
	assert_strequal( wstring_squeeze( string8 )->cstring, " heiße Sägezähne " );
}

//---------------------------------------------------------------------------------

void Test_wstring_center()
{
	autoWString *string1 = wstring_dup( "" );
	assert_strequal( wstring_center( string1, 1 )->cstring, " " );

	autoWString *string2 = wstring_dup( "" );
	assert_strequal( wstring_center( string2, 2 )->cstring, "  " );

	autoWString *string3 = wstring_dup( "a" );
	assert_strequal( wstring_center( string3, 1 )->cstring, "a" );

	autoWString *string4 = wstring_dup( "a" );
	assert_strequal( wstring_center( string4, 2 )->cstring, "a " );

	autoWString *string5 = wstring_dup( "a" );
	assert_strequal( wstring_center( string5, 3 )->cstring, " a " );

	autoWString *string6 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string6, 3 )->cstring, "weiß" );

	autoWString *string7 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string7, 4 )->cstring, "weiß" );

	autoWString *string8 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string8, 5 )->cstring, "weiß " );

	autoWString *string9 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string9, 6 )->cstring, " weiß " );

	autoWString *string10 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string10, 7 )->cstring, " weiß  " );

	autoWString *string11 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string11, 8 )->cstring, "  weiß  " );

	autoWString *string12 = wstring_dup( "weiß" );
	assert_strequal( wstring_center( string12, 12 )->cstring, "    weiß    " );
}
void Test_wstring_ljust()
{
	autoWString* string1 = wstring_dup( "" );
	assert_strequal( wstring_ljust( string1, 1 )->cstring, " " );

	autoWString* string2 = wstring_dup( "" );
	assert_strequal( wstring_ljust( string2, 2 )->cstring, "  " );

	autoWString* string3 = wstring_dup( "A" );
	assert_strequal( wstring_ljust( string3, 1 )->cstring, "A" );

	autoWString* string4 = wstring_dup( "A" );
	assert_strequal( wstring_ljust( string4, 2 )->cstring, " A" );

	autoWString* string5 = wstring_dup( "A" );
	assert_strequal( wstring_ljust( string5, 5 )->cstring, "    A" );

	autoWString* string6 = wstring_dup( "123456789" );
	assert_strequal( wstring_ljust( string6, 5 )->cstring, "123456789" );

	autoWString* string7 = wstring_dup( "ÄÖÜß" );
	assert_strequal( wstring_ljust( string7, 5 )->cstring, " ÄÖÜß" );

	autoWString* string8 = wstring_dup( "  TestÄÖÜß  " );
	assert_strequal( wstring_ljust( string8, 5 )->cstring, "  TestÄÖÜß  " );

	autoWString* string9 = wstring_dup( "  TestÄÖÜß  " );
	assert_strequal( wstring_ljust( string9, 15 )->cstring, "     TestÄÖÜß  " );
}
void Test_wstring_rjust()
{
	autoWString* string1 = wstring_dup( "" );
	assert_strequal( wstring_rjust( string1, 1 )->cstring, " " );

	autoWString* string2 = wstring_dup( "" );
	assert_strequal( wstring_rjust( string2, 2 )->cstring, "  " );

	autoWString* string3 = wstring_dup( "A" );
	assert_strequal( wstring_rjust( string3, 1 )->cstring, "A" );

	autoWString* string4 = wstring_dup( "A" );
	assert_strequal( wstring_rjust( string4, 2 )->cstring, "A " );

	autoWString* string5 = wstring_dup( "A" );
	assert_strequal( wstring_rjust( string5, 5 )->cstring, "A    " );

	autoWString* string6 = wstring_dup( "123456789" );
	assert_strequal( wstring_rjust( string6, 5 )->cstring, "123456789" );

	autoWString* string7 = wstring_dup( "ÄÖÜß" );
	assert_strequal( wstring_rjust( string7, 5 )->cstring, "ÄÖÜß " );

	autoWString* string8 = wstring_dup( "  TestÄÖÜß  " );
	assert_strequal( wstring_rjust( string8, 5 )->cstring, "  TestÄÖÜß  " );

	autoWString* string9 = wstring_dup( "  TestÄÖÜß  " );
	assert_strequal( wstring_rjust( string9, 15 )->cstring, "  TestÄÖÜß     " );
}

//---------------------------------------------------------------------------------

void Test_wstring_truncate()
{
	autoWString *string1 = wstring_dup( "" );
	assert_strequal( wstring_truncate( string1, 0 )->cstring, "" );
	assert_strequal( wstring_truncate( string1, 10 )->cstring, "" );
	assert_strequal( wstring_truncate( string1, 1000 )->cstring, "" );

	autoWString *string2 = wstring_dup( "A" );
	assert_strequal( wstring_truncate( string2, 1000 )->cstring, "A" );
	assert_strequal( wstring_truncate( string2, 10 )->cstring, "A" );
	assert_strequal( wstring_truncate( string2, 1 )->cstring, "A" );
	assert_strequal( wstring_truncate( string2, 0 )->cstring, "" );

	autoWString *string3 = wstring_dup( "Test" );
	assert_strequal( wstring_truncate( string3, 1000 )->cstring, "Test" );
	assert_strequal( wstring_truncate( string3, 4 )->cstring, "Test" );
	assert_strequal( wstring_truncate( string3, 3 )->cstring, "Tes" );

	autoWString *string4 = wstring_dup( "Howard Carpendale" );
	assert_strequal( wstring_truncate( string4, 6 )->cstring, "Howard" );
	assert_strequal( wstring_truncate( string4, 1 )->cstring, "H" );
}

//---------------------------------------------------------------------------------

static void concatTokens( const WString* token, void* data ) {
	wstring_append( data, token );
}
void Test_wstring_split_word()
{
	autoWString* result = wstring_dup("");

	autoWString *string1 = wstring_dup( "Test" );
	wstring_split( string1, " ", concatTokens, result );
	assert_strequal( result->cstring, "Test" );
	wstring_delete( &result );

	result = wstring_dup("");
	autoWString *string2 = wstring_dup( " Test" );
	wstring_split( string2, " ", concatTokens, result );
	assert_strequal( result->cstring, "Test" );
	wstring_delete( &result );

	result = wstring_dup("");
	autoWString *string3 = wstring_dup( " Test " );
	wstring_split( string3, " ", concatTokens, result );
	assert_strequal( result->cstring, "Test" );
	wstring_delete( &result );

	result = wstring_dup("");
	autoWString *string4 = wstring_dup( " Test " );
	wstring_split( string4, ",", concatTokens, result );
	assert_strequal( result->cstring, " Test " );
	wstring_delete( &result );

	result = wstring_dup("");
	autoWString *string5 = wstring_dup( "" );
	wstring_split( string5, ",", concatTokens, result );
	assert_strequal( result->cstring, "" );
	wstring_delete( &result );
}
void Test_wstring_split_fullSentence()
{
	autoWString* result = wstring_dup("");

	autoWString *string1 = wstring_dup( "Alle meine Entchen schwimmen auf dem See, schwimmen auf dem See." );
	wstring_split( string1, " ,.", concatTokens, result );
	assert_strequal( result->cstring, "AllemeineEntchenschwimmenaufdemSeeschwimmenaufdemSee" );
}
void Test_wstring_split_noSubstringsFound()
{
	autoWString* result = wstring_dup("");

	autoWString *string1 = wstring_dup( "Alle meine Entchen schwimmen auf dem See, schwimmen auf dem See." );
	wstring_split( string1, "!", concatTokens, result );
	assert_strequal( result->cstring, "Alle meine Entchen schwimmen auf dem See, schwimmen auf dem See." );
}

//---------------------------------------------------------------------------------

void Test_wstring_replaceAll_simple()
{
	autoWString *string1 = wstring_dup( "This is a test string." );
	wstring_replaceAll( string1, "test", "longer test" );
	assert_strequal( string1->cstring, "This is a longer test string." );

	autoWString *string2 = wstring_dup( "This is a\ntest\nstring." );
	wstring_replaceAll( string2, "a\ntest\n", "a test " );
	assert_strequal( string2->cstring, "This is a test string." );

	autoWString *string3 = wstring_dup( "Test Test Test" );
	wstring_replaceAll( string3, "Test", "Tester" );
	assert_strequal( string3->cstring, "Tester Tester Tester" );

	autoWString *string4 = wstring_dup( "This is a test string." );
	wstring_replaceAll( string4, "cowboy", "cowgirl" );
	assert_strequal( string4->cstring, "This is a test string." );
}

//---------------------------------------------------------------------------------

void
Test_wstring_toInt()
{
	autoWString* string1 = s.dup("");
	assert_equal( s.toInt( string1 ), INT_MIN );

	autoWString* string2 = s.dup("abc");
	assert_equal( s.toInt( string2 ), INT_MIN );

	autoWString* string3 = s.dup("1abc");
	assert_equal( s.toInt( string3 ), 1 );

	autoWString* string4 = s.dup("1abc2");
	assert_equal( s.toInt( string4 ), 1 );

	autoWString* string5 = s.dup("1234");
	assert_equal( s.toInt( string5 ), 1234 );

	autoWString* string6 = s.dup("-1234");
	assert_equal( s.toInt( string6 ), -1234 );

	autoWString* string7 = s.dup("0");
	assert_equal( s.toInt( string7 ), 0 );

	autoWString* string8 = s.dup("   -1234   ");
	assert_equal( s.toInt( string8 ), -1234 );
}
void
Test_wstring_toDouble()
{
	autoWString* string1 = s.dup("");
	assert_true( isnan( s.toDouble( string1 )));

	autoWString* string2 = s.dup("abc");
	assert_true( isnan( s.toDouble( string2 )));

	autoWString* string3 = s.dup("1abc");
	assert_dequal( s.toDouble( string3 ), 1.0 );

	autoWString* string4 = s.dup("1.abc2");
	assert_dequal( s.toDouble( string4 ), 1.0 );

	autoWString* string5 = s.dup("1234.567");
	assert_dequal( s.toDouble( string5 ), 1234.567 );

	autoWString* string6 = s.dup("-1234.567");
	assert_dequal( s.toDouble( string6 ), -1234.567 );

	autoWString* string7 = s.dup("0.0");
	assert_dequal( s.toDouble( string7 ), 0.0 );

	autoWString* string8 = s.dup("   -1234.567   ");
	assert_dequal( s.toDouble( string8 ), -1234.567 );
}

//---------------------------------------------------------------------------------

int main()
{
	printf( "\n" );

	testsuite( Test_wstring_newDup );
	testsuite( Test_wstring_printf );
	testsuite( Test_wstring_clone );
	testsuite( Test_wstring_stealCstring );

	testsuite( Test_wstring_compareCompareCaseEquals );
	testsuite( Test_wstring_similarity );

	testsuite( Test_wstring_append );
	testsuite( Test_wstring_appendf );
	testsuite( Test_wstring_prepend );

	testsuite( Test_wstring_ltrim );
	testsuite( Test_wstring_rtrim );
	testsuite( Test_wstring_trim );

	testsuite( Test_wstring_squeeze );

	testsuite( Test_wstring_center );
	testsuite( Test_wstring_ljust );
	testsuite( Test_wstring_rjust );

	testsuite( Test_wstring_truncate );

	testsuite( Test_wstring_split_word );
	testsuite( Test_wstring_split_fullSentence );
	testsuite( Test_wstring_split_noSubstringsFound );

	testsuite( Test_wstring_replaceAll_simple );

	testsuite( Test_wstring_toInt );
	testsuite( Test_wstring_toDouble );

	printf( "\n" );
	printf( "----------------------------\n" );
	printf( "| Tests  | Failed | Passed |\n" );
	printf( "| %-6zu | %-6zu | %-6zu |\n", testsFailed+testsPassed, testsFailed, testsPassed );
	printf( "----------------------------\n" );
}

//---------------------------------------------------------------------------------
