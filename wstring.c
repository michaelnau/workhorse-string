#define _GNU_SOURCE
#include "wstring.h"
#include <assert.h>
#include <errno.h>
#include <float.h>	//DBL_MIN, DBL_MAX
#include <ctype.h>
#include <wctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iso646.h>

//---------------------------------------------------------------------------------

static void
resize( WString* string, size_t newCapacity );

//static uint32_t
//utf8NextChar( char** str );

//static bool
//utf8MoreChars( const char* str );

static size_t
utf8len( const char *str );

static size_t
occurrences( const char *string, const char *search );

//---------------------------------------------------------------------------------
//	Memeory management & helpers
//---------------------------------------------------------------------------------

#define __wmax( x, y )	((x) > (y) ? (x) : (y))

#define __wxnew( type, ... )	\
	memcpy( __wxmalloc( sizeof( type )), &(type){ __VA_ARGS__ }, sizeof(type) )

void
__wdie( const char* text )
{
	fputs( text, stderr );
	abort();
}

void*
__wxmalloc( size_t size )
{
	void* ptr = malloc( size );
	if ( ptr ) return ptr;

	__wdie( "Out of memory." );
	return NULL;
}

void*
__wxrealloc( void* pointer, size_t size )
{
	void* ptr = realloc( pointer, size );
	if ( ptr ) return ptr;

	__wdie( "Out of memory." );
	return NULL;
}

void*
__wxcalloc( size_t number, size_t size )
{
	void* ptr = calloc( number, size );
	if ( ptr ) return ptr;

	__wdie( "Out of memory." );
	return NULL;
}

char*
__wstr_dup( const char* string )
{
	if ( not string ) string = "";

	size_t size = strlen( string ) + 1;
	char* copy = __wxmalloc( size );
    memcpy( copy, string, size );

    assert( copy );
    return copy;
}

static int
__wxvsnprintf( char* string, size_t size, const char* format, va_list args )
{
	assert( format );

	int len = vsnprintf( string, size, format, args );
	if ( len >= 0 ) return len;

	__wdie( "Out of memory." );
	return -1;
}

//Like __wstr_printf(), but takes a va_list argument
static char*
__wstr_printfva( const char* format, va_list args )
{
	assert( format );

    va_list args_copy;
	va_copy( args_copy, args );

	int len = __wxvsnprintf( NULL, 0, format, args_copy ) + 1;
	char* other = __wxmalloc( len );
	__wxvsnprintf( other, len, format, args );

	va_end( args_copy );

	assert( other );
	return other;
}


//---------------------------------------------------------------------------------

static WString*
checkString( const WString* string ) {
	assert( string->cstring );
	assert( string->size <= string->sizeBytes );
	assert( string->sizeBytes <= string->capacity );
	assert( string->sizeBytes == strlen( string->cstring ) + 1 );
	assert( string->size == utf8len( string->cstring ));

	return (WString*)string;
}

enum WStringConfiguration {
	WStringGrowthRate			= 2,
	WStringDefaultCapacity 		= 100,
	Utf8MaximumCharacterSize	= 4,
};


//const ElementType* stringElement = &(ElementType){
//	.clone = (ElementClone*)wstring_clone,
//	.delete = (ElementDelete*)wstring_delete,
//	.compare = (ElementCompare*)wstring_compare,
//	.fromString = (ElementFromString*)Array_fromString,
//	.toString = (ElementToString*)Array_toString
//};

//---------------------------------------------------------------------------------

//TODO: Step over UTF8 characters
#define wstring_foreachIndex( string, index, ... )					\
do {																\
	assert( string != NULL );										\
	char *__string = string->cstring;								\
	for ( size_t index = 0; __string[index] != '\0'; index++ ) {	\
		__VA_ARGS__													\
	}																\
}while( 0 )

//TODO: Step over UTF8 characters
#define str_foreachIndex( str, index, ... )					\
do {														\
	assert( str );											\
	for ( size_t index = 0; str[index] != 0; index++ ) {	\
		__VA_ARGS__											\
	}														\
}while(0)

//---------------------------------------------------------------------------------

WString*
wstring_new( const char* cstring, size_t capacity )
{
	assert( cstring );

	WString* string = __wxnew( WString,
		.size = utf8len( cstring ),
		.sizeBytes = strlen( cstring ) + 1,
		.capacity = capacity ? capacity : WStringDefaultCapacity,
	);

	string->capacity = __wmax( string->capacity, string->sizeBytes );

	//Copy the given char* into our new String*
	string->cstring = __wxmalloc( string->capacity );
	strcpy( string->cstring, cstring ),

	//Always terminate the cstring
	string->cstring[string->capacity - 1] = '\0';

	assert( string );
	return checkString( string );
}

WString*
wstring_dup( const char* cstring )
{
	return wstring_new( cstring, strlen( cstring )+1 );
}

WString*
wstring_clone( const WString* string )
{
	assert( string != NULL );

	WString* clone = wstring_new( string->cstring, string->capacity );

	assert( clone );
	assert( wstring_equals( clone, string ) );
	return checkString( clone );
}

void
wstring_clear( WString* string )
{
	assert( string );

	string->cstring[0] = '\0';
	string->size = 0;
	string->sizeBytes = 1;

	assert( wstring_empty( string ) );
	checkString( string );
}

char*
wstring_steal( WString** stringPtr )
{
	assert( stringPtr );
	assert( *stringPtr );

	WString* string = *stringPtr;

	char* stolen = string->cstring;
	string->cstring = NULL;
	wstring_delete( stringPtr );

	assert( stolen );
	return stolen;
}

char*
wstring_cstring( const WString* string )
{
	assert( string );

	return __wstr_dup( string->cstring );
}

void
wstring_delete( WString** stringPtr )
{
	if ( stringPtr == NULL or *stringPtr == NULL )
		return;

	WString* string = *stringPtr;

	free( string->cstring );
	free( *stringPtr );
	*stringPtr = NULL;
}

void
wstring_assign( WString** stringPtr, WString* other )
{
	if ( not stringPtr ) return;

	wstring_delete( stringPtr );
	*stringPtr = other;
}

//---------------------------------------------------------------------------------

WString*
wstring_append( WString* string, const WString* other )
{
	assert( string != NULL );
	assert( other != NULL );

	size_t newSize = string->sizeBytes + other->sizeBytes - 1;
	resize( string, newSize );

	strcpy( &string->cstring[string->sizeBytes - 1], other->cstring );
	string->size += other->size;
	string->sizeBytes += other->sizeBytes - 1;

	assert( string );
	assert( wstring_endsWith( string, other ));
	return checkString( string );
}

WString*
wstring_appendc( WString* string, const char* other )
{
	assert( string );
	assert( other );

	size_t sizeOther = strlen( other );
	size_t newSize = string->sizeBytes + sizeOther;
	resize( string, newSize );

	strcpy( &string->cstring[string->sizeBytes - 1], other );
	string->size += utf8len( other );
	string->sizeBytes += sizeOther;

	assert( string );
	return checkString( string );
}

WString*
wstring_appendn( WString* string, size_t n, const char* buffer )
{
	assert( string );
	assert( n > 0 );
	assert( buffer );

	size_t newSize = string->sizeBytes + n;
	resize( string, newSize );

	memcpy( &string->cstring[string->sizeBytes - 1], buffer, n );
	string->cstring[newSize-1] = 0;
	string->sizeBytes = newSize;
	string->size = utf8len( string->cstring );

	assert( string );
	return checkString( string );
}

WString*
wstring_appendf( WString* string, const char* format, ... )
{
	assert( string );
	assert( format );

    va_list args;
    va_start( args, format );
	char* other = __wstr_printfva( format, args );
	va_end( args );

	wstring_appendc( string, other );

	free( other );
	assert( string );
    return checkString( string );
}

WString*
wstring_printf( const char* format, ... )
{
	assert( format );

    va_list args;
    va_start( args, format );
	char* cstring  = __wstr_printfva( format, args );
	va_end( args );

	WString* string = wstring_dup( cstring );

	free( cstring );
	assert( string );
	return checkString( string );
}

WString*
wstring_prepend( WString* string, const WString* other )
{
	assert( string != NULL );
	assert( other != NULL );

	size_t newSize = string->sizeBytes + other->sizeBytes - 1;
	resize( string, newSize );

	memmove( &string->cstring[other->sizeBytes - 1], string->cstring, string->sizeBytes );
	memmove( string->cstring, other->cstring, other->sizeBytes - 1 );

	string->size += other->size;
	string->sizeBytes += other->sizeBytes - 1;

	assert( string );
	assert( wstring_startsWith( string, other ) );
	return checkString( string );
}

//REFACTOR: wstring_replace() must use resize()
static WString*
_replace( WString* string, const char *search, const char *replace, bool all )
{
	assert( string );
	assert( search );
	assert( replace );

	size_t count = occurrences( string->cstring, search );
	if ( count == 0 ) return checkString( string );

	size_t searchLen = strlen( search );
	size_t replaceLen = strlen( replace );

	size_t newSizeBytes = (
        string->sizeBytes - 1
      - searchLen * count
      + replaceLen * count
	) + 1;

	size_t newCapacity = __wmax( string->capacity, newSizeBytes );
	char *newCString = __wxcalloc( newCapacity, 1 );

	char *pos = string->cstring;
	char *current;
	bool repeat = true;
	while (( current = strstr( pos, search )) and repeat ) {
		size_t len = current - pos;
		strncat( newCString, pos, len );
		strncat( newCString, replace, replaceLen );
		pos = current + searchLen;
		repeat = all;
	}

	if ( pos != ( string->cstring + string->sizeBytes - 1 ))
		strncat( newCString, pos, ( string->cstring - pos ));

	free( string->cstring );
	string->cstring = newCString;
	string->sizeBytes = strlen( string->cstring ) + 1;
	string->size = utf8len( string->cstring );
	string->capacity = newCapacity;
	string->cstring[newCapacity - 1] = 0;

	assert( string != NULL );
	return checkString( string );
}

WString*
wstring_replace( WString* string, const char* search, const char* replace )
{
	return _replace( string, search, replace, false );
}

WString*
wstring_replaceAll( WString* string, const char* search, const char* replace )
{
	return _replace( string, search, replace, true );
}

inline static bool
isTrimmable( char character, const char *trimlist )
{
	for ( ; *trimlist != 0; trimlist++ ) {
		if ( *trimlist == character )
			return true;
	}

	return false;
}

WString*
wstring_trim( WString* string, const char chars[] )
{
	wstring_ltrim( string, chars );
	wstring_rtrim( string, chars );

	return string;
}

WString*
wstring_ltrim( WString* string, const char chars[] )
{
	assert( string );
	assert( chars );
	assert( chars[0] );

	char *newString = string->cstring;
	size_t newLen = string->sizeBytes;

	while ( isTrimmable( *newString, chars ) ) {
		newString++;
		newLen--;
		string->size--;
		string->sizeBytes--;
	}

	if ( newLen > 0 )
		memmove( string->cstring, newString, newLen );

	assert( string != NULL );
	return checkString( string );
}

WString*
wstring_rtrim( WString* string, const char chars[] )
{
	assert( string );
	assert( chars );
	assert( chars[0] );

	if ( string->sizeBytes < 2 )
		return string;

	char *newEnd = &string->cstring[string->sizeBytes - 2];

	while ( newEnd > string->cstring and isTrimmable( *newEnd, chars ) ) {
		newEnd--;
		string->size--;
		string->sizeBytes--;
	}

	newEnd[1] = '\0';

	assert( string != NULL );
	return checkString( string );
}

WString*
wstring_squeeze( WString* string )
{
	assert( string != NULL );

    if ( string->sizeBytes <= 1 )
		return checkString( string );

    char *from = string->cstring + 1;
    char *to = string->cstring + 1;

	while ( *from ) {
		char byte = *from;
		if ( isspace( byte ) and byte == *( from - 1 ) ) {
			string->size--;
			string->sizeBytes--;
		}
		else
			*to++ = byte;

		from++;
	}

	*to = 0;

	assert( string != NULL );
	return checkString( string );
}

WString*
wstring_truncate( WString* string, size_t size )
{
	assert( string != NULL );

	if ( size >= string->size )
		return checkString( string );

	size_t charIndex = 1;
	wstring_foreachIndex( string, byteIndex,
		if ( charIndex > size ) {
			string->cstring[byteIndex] = 0;
			string->size = size;
			string->sizeBytes = byteIndex + 1;
			return checkString( string );
		}
		charIndex++;
	);

	assert( string != NULL );
	assert( wstring_size( string ) <= size );
	return checkString( string );
}

WString*
wstring_center( WString* string, size_t size )
{
	assert( string != NULL );
	assert( size > 0 );

	if ( string->size >= size )
		return checkString( string );

	size_t delta = size - string->size;
	resize( string, string->sizeBytes + delta );

	size_t numberSpacesLeft = ( size - string->size ) / 2;
	size_t numberSpacesRight = size - string->size - numberSpacesLeft;

	//Move the old string and fill up with spaces
	memmove( &string->cstring[numberSpacesLeft], string->cstring, string->sizeBytes );
	memset( string->cstring, ' ', numberSpacesLeft );
	memset( &string->cstring[string->sizeBytes + numberSpacesLeft - 1], ' ', numberSpacesRight );

	string->size = size;
	string->sizeBytes += ( numberSpacesLeft + numberSpacesRight );

	string->cstring[string->sizeBytes - 1] = 0;

	assert( string != NULL );
	assert( wstring_size( string ) >= size );
	return checkString( string );
}

WString*
wstring_ljust( WString* string, size_t size )
{
	assert( string );
	assert( size > 0 );

	if ( string->size >= size ) return string;
	size_t delta = size - string->size;

	resize( string, string->sizeBytes + delta );
	memmove( &string->cstring[delta], string->cstring, string->sizeBytes );
	memset( string->cstring, ' ', delta );

	string->sizeBytes += delta;
	string->size = size;

	assert( string );
	assert( wstring_size( string ) >= size );
	return checkString( string );
}

WString*
wstring_rjust( WString* string, size_t size )
{
	assert( string );
	assert( size > 0 );

	if ( string->size >= size ) return string;
	size_t delta = size - string->size;

	resize( string, string->sizeBytes + delta );
	memset( &string->cstring[string->sizeBytes-1], ' ', delta );

	string->sizeBytes += delta;
	string->size = size;
	string->cstring[string->sizeBytes-1] = 0;

	assert( string );
	assert( wstring_size( string ) >= size );
	return checkString( string );
}

//---------------------------------------------------------------------------------

static void
wstring_map( WString* string, wint_t callback( wint_t ))
{
	//Convert string->cstring into a wide string.
	wchar_t* wideBuffer = __wxmalloc( (string->size+1) * sizeof(wchar_t) );
	size_t size = mbstowcs( wideBuffer, string->cstring, string->size + 1 );

	if ( size != (size_t)-1 ) {
		free( string->cstring );

		//Apply the callback function.
		for ( size_t i = 0; i < string->size; i++ )
			wideBuffer[i] = callback( wideBuffer[i] );

		//Reconvert it to a char* string.
		string->capacity = string->size * Utf8MaximumCharacterSize + 1;
		string->cstring = __wxmalloc( string->capacity );
		string->sizeBytes = wcstombs( string->cstring, wideBuffer, string->capacity )+1;
		assert( string->sizeBytes != (size_t)-1 && "A bug in mbstowcs(), callback() or wcstombs() occurred." );
		string->size = utf8len( string->cstring );
		string->cstring[ string->sizeBytes-1 ] = 0;
	}

	//Clean up.
	free( wideBuffer );
}

WString*
wstring_toLower( WString* string )
{
	assert( string );

	wstring_map( string, towlower );

	assert( string );
	return checkString( string );
}

WString*
wstring_toUpper( WString* string )
{
	assert( string );

	wstring_map( string, towupper );

	assert( string );
	return checkString( string );
}

//---------------------------------------------------------------------------------

size_t
wstring_size( const WString* string )
{
	assert( string != NULL );

	return string->size;
}

size_t
wstring_sizeBytes( const WString* string )
{
	assert( string != NULL );

	return string->sizeBytes;
}

bool
wstring_empty( const WString* string )
{
	assert( string != NULL );

	return string->size == 0;
}

bool
wstring_equals( const WString* string, const WString *other )
{
	assert( string != NULL );
	assert( other != NULL );

	return string->size == other->size and					//Faster comparison for unequal strings
		   string->sizeBytes == other->sizeBytes and			//Faster comparison for unequal strings
		   strcmp( string->cstring, other->cstring ) == 0;
}

int
wstring_compare( const WString* string, const WString *other )
{
	assert( string != NULL );
	assert( other != NULL );

	return strcmp( string->cstring, other->cstring );
}

//TODO: Only ASCII characters are compared correctly.
//TODO: Replace GNU strcasecmp() by own implementation
int
wstring_compareCase( const WString* string, const WString *other )
{
	assert( string != NULL );
	assert( other != NULL );

	return strcasecmp( string->cstring, other->cstring );
}

//Taken and modified from https://github.com/wooorm/levenshtein.c, MIT licensed
size_t
wstring_similarity( const WString* string, const WString* other )
{
	assert( string );
	assert( other );

    size_t length = string->size;
    size_t bLength = other->size;

    //Shortcut optimizations / degenerate cases.
    if ( wstring_equals( string, other )) return 0;
    if ( length == 0 ) return bLength;
    if ( bLength == 0 ) return length;

    //initialize the vector.
    size_t index = 0;
    size_t* cache = calloc( length, sizeof( size_t ));
    while ( index < length ) {
        cache[index] = index + 1;
        index++;
    }

    size_t distance;
    size_t result;
    size_t bIndex = 0;
    while ( bIndex < bLength ) {
        char code = other->cstring[bIndex];
        result = distance = bIndex++;
        index = -1;

        while ( ++index < length ) {
            size_t bDistance = code == string->cstring[index] ? distance : distance + 1;
            distance = cache[index];

            cache[index] = result = distance > result
                ? bDistance > result
                    ? result + 1
                    : bDistance
                : bDistance > distance
                    ? distance + 1
                    : bDistance;
        }
    }

    free(cache);
    return result;
}

bool
wstring_contains( const WString* string, const WString *other )
{
	assert( string != NULL );
	assert( other != NULL );

	return strstr( string->cstring, other->cstring ) != NULL;
}

bool
wstring_startsWith( const WString* string, const WString *other )
{
	assert( string != NULL );
	assert( other != NULL );

	return strstr( string->cstring, other->cstring ) == string->cstring;
}

bool
wstring_endsWith( const WString* string, const WString *other )
{
	assert( string != NULL );
	assert( other != NULL );

	if ( string->sizeBytes < other->sizeBytes )
		return false;

	size_t endPosition = string->sizeBytes - other->sizeBytes;

	return strcmp( &string->cstring[endPosition], other->cstring ) == 0;
}

//---------------------------------------------------------------------------------

//TODO: Replace POSIX strtok_r() by own implementation

void

wstring_split( const WString* string, const char *delimiters, void foreach( const WString*, void* data ), void* data )
{
	assert( string );
	assert( delimiters and delimiters[0] );
	assert( foreach );

	//Get the first substring.
	char* copy = __wstr_dup( string->cstring );
	char* strtokPtr = NULL;
	char* token = copy[0] ? strtok_r( copy, delimiters, &strtokPtr ) : "";

	while ( token ) {
		WString* tokenString = wstring_dup( token );
			foreach( tokenString, data );
        wstring_delete( &tokenString );
		token = strtok_r( NULL, delimiters, &strtokPtr );
	}

	free( copy );
}

//---------------------------------------------------------------------------------

int
wstring_toInt( const WString* string )
{
	assert( string );

	char *endptr;

	errno = 0;
	long value = strtol( string->cstring, &endptr, 10 );

	if (( errno == ERANGE and ( value == LONG_MAX or value == LONG_MIN )) or ( errno != 0 and value == 0 ))
		return INT_MIN;

	if ( endptr == string->cstring )
		return INT_MIN;

	return value;
}

double
wstring_toDouble( const WString* string )
{
	assert( string );

	char *endptr;

	errno = 0;
	double value = strtod( string->cstring, &endptr );

	if (( errno == ERANGE and ( value == DBL_MAX or value == DBL_MIN )) or ( errno != 0 and value == 0.0 ))
		return NAN;

	if ( endptr == string->cstring )
		return NAN;

	return value;
}

//---------------------------------------------------------------------------------

//Resize the string to the given new capacity.
static void
resize( WString* string, size_t newCapacity )
{
	if ( newCapacity > string->capacity ) {
		string->capacity = __wmax( newCapacity, string->capacity * WStringGrowthRate );
		string->cstring = __wxrealloc( string->cstring, string->capacity );
		string->cstring[string->capacity-1] = 0;
	}
	assert( string->capacity >= newCapacity );
//	checkString( string );
}

#if 0
static uint32_t
utf8NextChar( char** strPtr )
{
	assert( strPtr );
	assert( *strPtr );
	const unsigned char* str = *(const unsigned char**)strPtr;

	uint32_t result;
	if (( *str & 0xf8) == 0xf0 ) {
		result = (str[0] << 24) + (str[1] << 16) + (str[2] << 8) + str[3];
		*strPtr += 4;
	}
	if (( *str & 0xf0 ) == 0xe0 ) {
		result = (str[0] << 16) + (str[1] << 8) + str[2];
		*strPtr += 3;
	}
	if (( *str & 0xe0 ) == 0xc0 ) {
		result = (str[0] << 8) + str[1];
		*strPtr += 2;
	}
	else {
		result = str[0];
		*strPtr += 1;
	}

	return result;
}
#endif // 0

#if 0
static bool
utf8MoreChars( const char* str )
{
	assert( str );

	if ( *str ) return true;
	else return false;
}
#endif

//Taken from Github, UTF8.h (Public Domain), then modified
static size_t
utf8len( const char* str )
{
  const unsigned char* s = (const unsigned char* )str;
  size_t length = 0;

  while ('\0' != *s) {
    if (0xf0 == (0xf8 & *s)) {
      // 4-byte utf8 code point (began with 0b11110xxx)
      s += 4;
    } else if (0xe0 == (0xf0 & *s)) {
      // 3-byte utf8 code point (began with 0b1110xxxx)
      s += 3;
    } else if (0xc0 == (0xe0 & *s)) {
      // 2-byte utf8 code point (began with 0b110xxxxx)
      s += 2;
    } else { // if (0x00 == (0x80 & *s)) {
      // 1-byte ascii (began with 0b0xxxxxxx)
      s += 1;
    }

    // no matter the bytes we marched s forward by, it was
    // only 1 utf8 codepoint
    length++;
  }

  return length;
}

/*Algorithm taken from the Github account from Stephen Mathieson, then modified.
*/
static size_t
occurrences( const char *string, const char *search )
{
	assert( string );
	assert( search );

	const char *selfPosition = string;
	size_t count = 0;
	size_t searchLength = strlen( search );

	while ( ( selfPosition = strstr( selfPosition, search ) ) ) {
		selfPosition += searchLength;
		count++;
	}

	return count;
}

//---------------------------------------------------------------------------------
