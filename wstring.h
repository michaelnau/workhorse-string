/**	@file
	Dynamically growing C strings with the power and ease of a scripting language.
*/
#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include <limits.h>		//INT_MIN
#include <math.h>		//NAN
#include <stdio.h>
#include <stdbool.h>	//bool
#include <stddef.h>		//size_t

//---------------------------------------------------------------------------------

/*	GCC Compiler hint to check the variadic function's arguments like printf.
*/
#define PRINTF( indexFormat, indexArgs ) __attribute__((format(printf, indexFormat, indexArgs )))

//---------------------------------------------------------------------------------
//	Types
//---------------------------------------------------------------------------------

/** String type that can grow when necessary. Supports many common operations
	like search, replace, compare, split or trim. Supports UTF-8 strings.
*/
typedef struct WString {
	size_t	size;		//<Private member: Do not use. Number of contained UTF8 characters excluding the 0 terminator
	size_t	sizeBytes;	//<Private member: Do not use. Number of contained bytes including the 0 terminator
	size_t	capacity;	//<Private member: Do not use. Maximum number of bytes including the 0 terminator. If sizeBytes > capacity, cstring must be realloced.
	char*	cstring;	///<Public member: A 0-terminated C string, may contain UTF8 characters.
}WString;

//---------------------------------------------------------------------------------
//	String creation and destruction
//---------------------------------------------------------------------------------

/**	Create a string from a C string and a given cmaximum apacity.
*/
WString*
wstring_new( const char* cstring, size_t capacity );

/**	Create a string from a C string.
*/
WString*
wstring_dup( const char* cstring );

/**	Makes a deep copy of a string.
*/
WString*
wstring_clone( const WString* string );

/**	Create a new string from a format string.

	@param format
	@param ...
	@return
*/
WString*
wstring_printf( const char* format, ... ) PRINTF(1, 2);

/**	Destroys a string.
*/
void
wstring_delete( WString** stringPointer );

/**	Sets the string text to "".
*/
void
wstring_clear( WString* string );

/**	Destroys the string and returns the contained char*.

	@param stringPointer
	@return The contained char*
*/
char*
wstring_steal( WString** stringPointer );

/**	Returns a copy of the contained char*.

	@param string
	@return The copied char*
*/
char*
wstring_cstring( const WString* string );

/**	Destroys a string and assigns another string to its pointer.
*/
void
wstring_assign( WString** stringPointer, WString* other );

//---------------------------------------------------------------------------------

/**	Return the number of UTF8 characters
*/
size_t
wstring_size( const WString* string );

/**	Return the number of bytes including the 0 terminator.
*/
size_t
wstring_sizeBytes( const WString* string );

/**	Check if the string is empty.
*/
bool
wstring_empty( const WString* string );

/**	Check if the string holds at least one character.
*/
static inline bool
wstring_nonEmpty( const WString* string ) { return !wstring_empty( string ); }

/**	Check if two strings are equal.

	@param string
	@param other
	@return
*/
bool
wstring_equals( const WString* string, const WString* other );

/**	Compare two strings with each other.

	@param string
	@param other
	@return
*/
int
wstring_compare( const WString* string, const WString* other );

/**	Compare two strings with each other ignoring case differences.

	@param string
	@param other
	@return
*/
int
wstring_compareCase( const WString* string, const WString* other );

/**	Compare two strings and return a measure for their similarity.

	Uses the Levenshtein algorithm. The strings are compared byte-wise, so UTF-8 strings
	might not get good results.

	@param string
	@param other
	@return 0 for identity and 1, 2, 3 etc. for the number of differences.
	@pre string != NULL
	@pre other != NULL
*/
size_t
wstring_similarity( const WString* string, const WString* other );

/**	Check if a string contains another string.

	@param string
	@param other
	@return
*/
bool
wstring_contains( const WString* string, const WString* other );

/**	Check if a string starts with another string.

	@param string
	@param other
	@return
*/
bool
wstring_startsWith( const WString* string, const WString* other );

/**	Check if a string ends with another string.

	@param string
	@param other
	@return
*/
bool
wstring_endsWith( const WString* string, const WString* other );

//---------------------------------------------------------------------------------

/**	Append a string to another string.

	@param string The string the other string gets appended to.
	@param other The appended string.
	@return string
*/
WString*
wstring_append( WString* string, const WString* other );

/**	Prepend a string before another string.
*/
WString*
wstring_prepend( WString* string, const WString* other );

/**	Append a C string to another string.

	@param string The string the other string gets appended to.
	@param other The appended string.
	@return string
*/
WString*
wstring_appendc( WString* string, const char* other );

/**	Appends n characters from a buffer to the string.

	@param string
	@param n
	@param buffer
	@return
*/
WString*
wstring_appendn( WString* string, size_t n, const char* buffer );

/**	Appends a printf-like string to another string.

	@param string The string the other string gets appended to.
	@param format Format string like in printf()
	@param ... one or more values according to the format string
	@return string
*/
WString*
wstring_appendf( WString* string, const char* format, ... ) PRINTF(2, 3) ;

//String*
//TODO: wstring_insert( String* string, size_t position );

//---------------------------------------------------------------------------------

//uint32_t
//TODO: wstring_at( const String* string, size_t position );

//String*
//TODO: wstring_slice( const String* string, size_t start, size_t end );


//---------------------------------------------------------------------------------

/**	Replaces the first match of a search string with a replacement string.

	@param string
	@param search
	@param replace
	@return
*/
WString*
wstring_replace( WString* string, const char* search, const char* replace );

/**	Replaces all matches of a search string with a replacement string.

	@param string
	@param search
	@param replace
	@return
*/
WString*
wstring_replaceAll( WString* string, const char* search, const char* replace );

//---------------------------------------------------------------------------------

/**	Remove characters from the start and end.

	@param string The string to be trimmed
	@param chars Array of characters to be trimmed away
	@return string, to allow chaining with other string functions
*/
WString*
wstring_trim( WString* string, const char chars[] );

/**	Remove characters from the start.
*/
WString*
wstring_ltrim( WString* string, const char chars[] );

/**	Remove characters from the end.
*/
WString*
wstring_rtrim( WString* string, const char chars[] );

/**	Remove all multiple whitespace characters from the string.

	@param string
	@return string
*/
WString*
wstring_squeeze( WString* string );

//---------------------------------------------------------------------------------

/**	Convert a string to all lower case characters.

	Uses internally the wchar_t standard library functions. So it basically works for
	non-ASCII characters with the known limitations of those functions.

	@param string The string to be converted
	@return The converted string
	@pre string != NULL
*/
WString*
wstring_toLower( WString* string );

/**	Convert a string to all upper case characters.

	Uses internally the wchar_t standard library functions. So it basically works for
	non-ASCII characters with the known limitations of those functions.

	@param string The string to be converted
	@return The converted string
	@pre string != NULL
*/
WString*
wstring_toUpper( WString* string );

/**	Convert the first letters of all words to uppercase and all other letters to lowercase.

	Uses internally the wchar_t standard library functions. So it basically works for
	non-ASCII characters with the known limitations of those functions.

	@param string The string to be converted
	@return The converted string
	@pre string != NULL
*/
WString*
wstring_toTitle( WString* string );

//---------------------------------------------------------------------------------

/**	Truncate a string after a given number of characters.

	Uses internally the wchar_t standard library functions. So it basically works for
	non-ASCII characters with the known limitations of those functions.

	@param string The string to be truncated
	@param size The maximum character number that will remain
	@return string, allowing to chain the function with other wstring_xyz() calls
*/
WString*
wstring_truncate( WString* string, size_t size );

//---------------------------------------------------------------------------------

/**	Centers a string in a possibly longer new string and fills it left and right
	with spaces.
*/
WString*
wstring_center( WString* string, size_t size );

/**	Increase the string to the given size by filling it with spaces at the front.
*/
WString*
wstring_ljust( WString* string, size_t size );

/**	Increase the string to the given size by filling it with spacess at the end.
*/
WString*
wstring_rjust( WString* string, size_t size );

//---------------------------------------------------------------------------------

/**	Split a string in substrings separated by the given delimiter characters.

	@param string The string to be split in tokens
	@param delimiters A list of one-character ASCII delimiters
	@param foreach A function to be called for each token
	@param data Optional data argument passed to the foreach() function

	Example:
	\code
	//Callback function to be called for each token
	void printNumber( const String* number, void* unused ) {
		printf( "%s\n", number->cstring );
	}
	void foo() {
		//Create a string with comma-separated numbers
		String* myString = wstring_new( "3.141, 2.791, 42, -1" );
		//Split the string in numbers.
		//Take " " and "," as separating characters
		//For each resulting token: print it.
		wstring_split( myString, " ,", printNumber, NULL );
	}
	\endcode
*/
void
wstring_split( const WString* string, const char* delimiters, void foreach( const WString*, void* data ), void* data );

//---------------------------------------------------------------------------------

/**	Parse a string and convert it to an integer.

	@param string
	@return The resulting integer or INT_MIN if the string could not be parsed.
*/
int
wstring_toInt( const WString* string );

/**	Parse a string and convert it to a double.

	@param string
	@return The resulting double or NAN if the string could not be parsed.
*/
double
wstring_toDouble( const WString* string );

//---------------------------------------------------------------------------------

#endif // STRING_H_INCLUDED
