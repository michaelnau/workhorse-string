/**	@file
	Dynamically growing strings with all common operations like append
	trim, split etc.
*/
#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

//#include "Basic.h"		//PRINTF
#include <limits.h>		//INT_MIN
#include <math.h>		//NAN
#include <stdio.h>
#include <stdbool.h>	//bool
#include <stddef.h>		//size_t

/*	Compiler hint to check the variadic function's arguments like printf.
*/
#define PRINTF( indexFormat, indexArgs ) __attribute__((format(printf, indexFormat, indexArgs )))
//---------------------------------------------------------------------------------
//	Types
//---------------------------------------------------------------------------------

/** String type that can grow when necessary. Supports many common operations
	like search, replace, compare, split or trim. Supports UTF-8 strings.
*/
typedef struct WString {
	size_t	size;		///<Private member: Do not use. Number of contained UTF8 characters
	size_t	sizeBytes;	///<Private member: Do not use. Number of contained bytes including the 0 terminator
	size_t	capacity;	///<Private member: Do not use. Maximum number of bytes. If sizeBytes > capacity, cstring must be realloced.
	char*	cstring;	///<Public member: A 0-terminated C string, may contain UTF8 characters.
	//TODO: bool static;	//String was created static or on the stack without allocation and may not be modified.
}WString;

/**	When a string is defined as autoString it gets destroyed automatically
	when leaving scope.
*/
#define autoWString __attribute__(( cleanup( String_delete ))) WString

//---------------------------------------------------------------------------------
//	String namespace
//---------------------------------------------------------------------------------

/**	Structure containing pointers to all String functions. Can be used to simulate
	a module namespace.

	Example:
	\code
    StringNamespace s = stringNamespace;

    ...

    String* string = s.new( "Test", 20 );
	\endcode
*/
typedef struct WStringNamespace {
	WString*	(*new)			(const char* cstring, size_t capacity);
	WString*	(*dup)			(const char*);
	WString*	(*clone)		(const WString*);
	WString*	(*printf)		(const char*, ...);
	void	(*delete)		(WString**);
	void	(*clear)		(WString*);
	char*	(*steal)		(WString**);
	void	(*assign)		(WString** string, WString* other);

	bool	(*empty)		(const WString*);
	bool	(*nonEmpty)		(const WString*);
	bool	(*equals)		(const WString*, const WString*);
	int		(*compare)		(const WString*, const WString*);
	int		(*compareCase)	(const WString*, const WString*);
	size_t	(*similarity)	(const WString*, const WString*);
	bool	(*contains)		(const WString*, const WString*);
	bool	(*startsWith)	(const WString*, const WString*);
	bool	(*endsWith)		(const WString*, const WString*);

	WString*	(*append)		(WString*, const WString*);
	WString*	(*appendc)		(WString*, const char*);
	WString*	(*appendn)		(WString*, size_t, const char*);
	WString*	(*appendf)		(WString*, const char*, ... );
	WString*	(*prepend)		(WString*, const WString*);

	WString*	(*replace)		(WString*, const char*, const char*);
	WString*	(*replaceAll)	(WString*, const char*, const char*);

	WString*	(*trim)			(WString*, const char[]);
	WString*	(*ltrim)		(WString*, const char[]);
	WString*	(*rtrim)		(WString*, const char[]);
	WString*	(*squeeze)		(WString*);

	WString*	(*truncate)		(WString*, size_t);
	WString*	(*center)		(WString*, size_t);
	WString*	(*ljust)		(WString*, size_t);
	WString*	(*rjust)		(WString*, size_t);

	void	(*split)		(const WString*, const char*, void foreach(const WString*));
	int		(*toInt)		(const WString*);
	double	(*toDouble)		(const WString*);
}WStringNamespace;

/**	Predefined value for StringNamespace variables
*/
#define wstringNamespace {				\
	.new = String_new,					\
	.dup = String_dup,					\
	.clone = String_clone,				\
	.printf = String_printf,			\
	.delete = String_delete,			\
	.clear = String_clear,				\
	.steal = String_steal,				\
	.assign = String_assign,			\
\
	.empty = String_empty,				\
	.nonEmpty = String_nonEmpty,		\
	.equals = String_equals,			\
	.compare = String_compare,			\
	.compareCase = String_compareCase,	\
	.similarity = String_similarity,	\
	.contains = String_contains,		\
	.startsWith = String_startsWith,	\
	.endsWith = String_endsWith,		\
\
	.append = String_append,			\
	.appendc = String_appendc,			\
	.appendn = String_appendn,			\
	.appendf = String_appendf,			\
	.prepend = String_prepend,			\
	.replace = String_replace,			\
	.replaceAll = String_replaceAll,	\
\
	.trim = String_trim,				\
	.ltrim = String_ltrim,				\
	.rtrim = String_rtrim,				\
	.squeeze = String_squeeze,			\
\
	.truncate = String_truncate,		\
	.center = String_center,			\
	.ljust = String_ljust,				\
	.rjust = String_rjust,				\
\
	.split = String_split,				\
	.toInt = String_toInt,				\
	.toDouble = String_toDouble,		\
}

//---------------------------------------------------------------------------------
//	String creation and destruction
//---------------------------------------------------------------------------------

/**	Create a new string.

	@param .string (const char*, default = "")
	@param .capacity (size_t, default = 100)
	@return (String*)
*/
#define WString( ... )					\
	__String( (WString){ .capacity = 100, __VA_ARGS__ })

WString*
__String( WString input );

/*	Create a non-allocated string in the current scope.
*/
#if 0
#define StringLocal( string )				\
	&(WString){								\
		.cstring = string,					\
		.size = utf8len( string ),			\
		.sizeBytes = strlen( string )+1,	\
		.capacity = strlen( string )+1,		\
	}
#endif

/**	Create a string from a C string and a given cmaximum apacity.
*/
WString*
String_new( const char* cstring, size_t capacity );

/**	Create a string from a C string.
*/
WString*
String_dup( const char* cstring );

/**	Makes a deep copy of a string.
*/
WString*
String_clone( const WString* string );

/**	Create a new string from a format string.

	@param format
	@param ...
	@return
*/
WString*
String_printf( const char* format, ... ) PRINTF(1, 2);

/**	Destroys a string.
*/
void
String_delete( WString** stringPointer );

/**	Sets the string text to "".
*/
void
String_clear( WString* string );

/**	Destroys the string and returns the contained char*.

	@param stringPointer
	@return The contained char*
*/
char*
String_steal( WString** stringPointer );

/**	Returns a copy of the contained char*.

	@param string
	@return The copied char*
*/
char*
String_cstring( const WString* string );

/**	Destroys a string and assigns another string to its pointer.
*/
void
String_assign( WString** stringPointer, WString* other );

//---------------------------------------------------------------------------------

/**	Return the number of UTF8 characters
*/
size_t
String_size( const WString* string );

/**	Return the number of bytes including the 0 terminator.
*/
size_t
String_sizeBytes( const WString* string );

/**	Check if the string is empty.
*/
bool
String_empty( const WString* string );

/**	Check if the string holds at least one character.
*/
static inline bool
String_nonEmpty( const WString* string ) { return !String_empty( string ); }

/**	Check if two strings are equal.

	@param string
	@param other
	@return
*/
bool
String_equals( const WString* string, const WString* other );

/**	Compare two strings with each other.

	@param string
	@param other
	@return
*/
int
String_compare( const WString* string, const WString* other );

/**	Compare two strings with each other ignoring case differences.

	@param string
	@param other
	@return
*/
int
String_compareCase( const WString* string, const WString* other );

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
String_similarity( const WString* string, const WString* other );

/**	Check if a string contains another string.

	@param string
	@param other
	@return
*/
bool
String_contains( const WString* string, const WString* other );

/**	Check if a string starts with another string.

	@param string
	@param other
	@return
*/
bool
String_startsWith( const WString* string, const WString* other );

/**	Check if a string ends with another string.

	@param string
	@param other
	@return
*/
bool
String_endsWith( const WString* string, const WString* other );

//---------------------------------------------------------------------------------

/**	Append a string to another string.

	@param string The string the other string gets appended to.
	@param other The appended string.
	@return string
*/
WString*
String_append( WString* string, const WString* other );

/**	Prepend a string before another string.
*/
WString*
String_prepend( WString* string, const WString* other );

/**	Append a C string to another string.

	@param string The string the other string gets appended to.
	@param other The appended string.
	@return string
*/
WString*
String_appendc( WString* string, const char* other );

/**	Appends n characters from a buffer to the string.

	@param string
	@param n
	@param buffer
	@return
*/
WString*
String_appendn( WString* string, size_t n, const char* buffer );

/**	Appends a printf-like string to another string.

	@param string The string the other string gets appended to.
	@param format Format string like in printf()
	@param ... one or more values according to the format string
	@return string
*/
WString*
String_appendf( WString* string, const char* format, ... ) PRINTF(2, 3) ;

//String*
//TODO: String_insert( String* string, size_t position );

//---------------------------------------------------------------------------------

//uint32_t
//TODO: String_at( const String* string, size_t position );

//String*
//TODO: String_slice( const String* string, size_t start, size_t end );

//---------------------------------------------------------------------------------

/**	Replaces the first match of a search string with a replacement string.

	@param string
	@param search
	@param replace
	@return
*/
WString*
String_replace( WString* string, const char* search, const char* replace );

/**	Replaces all matches of a search string with a replacement string.

	@param string
	@param search
	@param replace
	@return
*/
WString*
String_replaceAll( WString* string, const char* search, const char* replace );

//---------------------------------------------------------------------------------

/**	Remove characters from the start and end.

	@param string The string to be trimmed
	@param chars Array of characters to be trimmed away
	@return string, to allow chaining with other string functions
*/
WString*
String_trim( WString* string, const char chars[] );

/**	Remove characters from the start.
*/
WString*
String_ltrim( WString* string, const char chars[] );

/**	Remove characters from the end.
*/
WString*
String_rtrim( WString* string, const char chars[] );

/**	Remove all multiple whitespace characters from the string.

	@param string
	@return string
*/
WString*
String_squeeze( WString* string );

//---------------------------------------------------------------------------------

//String*
//TODO: String_toLower( String* string );

//String*
//TODO: String_toUpper( String* string );

//String*
//TODO: String_toTitle( String* string );

//---------------------------------------------------------------------------------

/**	Truncate a string after a given number of characters.

	@param string The string to be truncated
	@param size The maximum character number that will remain
	@return string, allowing to chain the function with other String_xyz() calls
*/
WString*
String_truncate( WString* string, size_t size );

//---------------------------------------------------------------------------------

/**	Centers a string in a possibly longer new string and fills it left and right
	with spaces.
*/
WString*
String_center( WString* string, size_t size );

/**	Increase the string to the given size by filling it with spaces at the front.
*/
WString*
String_ljust( WString* string, size_t size );

/**	Increase the string to the given size by filling it with spacess at the end.
*/
WString*
String_rjust( WString* string, size_t size );

//---------------------------------------------------------------------------------

/**	Split a string in substrings separated by the given delimiter characters.

	@param string The string to be split in tokens
	@param delimiters A list of one-character delimiters
	@param foreach A function to be called for each token

	Example:
	\code
	//Callback function to be called for each token
	void printNumber( const String* number ) {
		printf( "%s\n", number->cstring );
	}
	void foo() {
		//Create a string with comma-separated numbers
		String* myString = String_new( "3.141, 2.791, 42, -1" );
		//Split the string in numbers.
		//Take " " and "," as separating characters
		//For each resulting token: print it.
		String_split( myString, " ,", printNumber );
	}
	\endcode
*/
void
String_split( const WString* string, const char* delimiters, void foreach( const WString* ));

//---------------------------------------------------------------------------------

/**	Parse a string and convert it to an integer.

	@param string
	@return The resulting integer or INT_MIN if the string could not be parsed.
*/
int
String_toInt( const WString* string );

/**	Parse a string and convert it to a double.

	@param string
	@return The resulting double or NAN if the string could not be parsed.
*/
double
String_toDouble( const WString* string );

//---------------------------------------------------------------------------------

#endif // STRING_H_INCLUDED
