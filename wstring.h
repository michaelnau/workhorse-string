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
	size_t	size;		//<Private member: Do not use. Number of contained UTF8 characters
	size_t	sizeBytes;	//<Private member: Do not use. Number of contained bytes including the 0 terminator
	size_t	capacity;	//<Private member: Do not use. Maximum number of bytes. If sizeBytes > capacity, cstring must be realloced.
	char*	cstring;	///<Public member: A 0-terminated C string, may contain UTF8 characters.
}WString;

/**	When a string is defined as autoString it gets destroyed automatically
	when leaving scope.
*/
#define autoWString __attribute__(( cleanup( wstring_delete ))) WString

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

	void	(*split)		(const WString*, const char*, void foreach(const WString*, void* data), void* data);
	int		(*toInt)		(const WString*);
	double	(*toDouble)		(const WString*);
}WStringNamespace;

/**	Predefined value for StringNamespace variables
*/
#define wstringNamespace {				\
	.new = wstring_new,					\
	.dup = wstring_dup,					\
	.clone = wstring_clone,				\
	.printf = wstring_printf,			\
	.delete = wstring_delete,			\
	.clear = wstring_clear,				\
	.steal = wstring_steal,				\
	.assign = wstring_assign,			\
\
	.empty = wstring_empty,				\
	.nonEmpty = wstring_nonEmpty,		\
	.equals = wstring_equals,			\
	.compare = wstring_compare,			\
	.compareCase = wstring_compareCase,	\
	.similarity = wstring_similarity,	\
	.contains = wstring_contains,		\
	.startsWith = wstring_startsWith,	\
	.endsWith = wstring_endsWith,		\
\
	.append = wstring_append,			\
	.appendc = wstring_appendc,			\
	.appendn = wstring_appendn,			\
	.appendf = wstring_appendf,			\
	.prepend = wstring_prepend,			\
	.replace = wstring_replace,			\
	.replaceAll = wstring_replaceAll,	\
\
	.trim = wstring_trim,				\
	.ltrim = wstring_ltrim,				\
	.rtrim = wstring_rtrim,				\
	.squeeze = wstring_squeeze,			\
\
	.truncate = wstring_truncate,		\
	.center = wstring_center,			\
	.ljust = wstring_ljust,				\
	.rjust = wstring_rjust,				\
\
	.split = wstring_split,				\
	.toInt = wstring_toInt,				\
	.toDouble = wstring_toDouble,		\
}

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

//String*
//TODO: wstring_toLower( String* string );

//String*
//TODO: wstring_toUpper( String* string );

//String*
//TODO: wstring_toTitle( String* string );

//---------------------------------------------------------------------------------

/**	Truncate a string after a given number of characters.

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
		String* myString = wstring_new( "3.141, 2.791, 42, -1" );
		//Split the string in numbers.
		//Take " " and "," as separating characters
		//For each resulting token: print it.
		wstring_split( myString, " ,", printNumber );
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
