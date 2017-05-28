#define _GNU_SOURCE
#include "wstring.h"
#include <assert.h>
#include <errno.h>
#include <float.h>	//DBL_MIN, DBL_MAX
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

//---------------------------------------------------------------------------------

static void
resize( WString *self, size_t newCapacity );

size_t
utf8len( const char *str );

static size_t
occurrences( const char *self, const char *search );

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

//---------------------------------------------------------------------------------

static WString*
checkString( const WString* string ) {
	assert( string->cstring != NULL );
	assert( string->size <= string->sizeBytes );
	assert( string->sizeBytes <= string->capacity );
	assert( string->size == utf8len( string->cstring ));
	assert( string->sizeBytes == strlen( string->cstring ) + 1 );

	return (WString*)string;
}

enum WStringConfiguration {
	WStringGrowthRate	= 2,
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
#define wstring_foreachIndex( self, index, ... )							\
do {																	\
	assert( self != NULL );											\
	char *__string = self->cstring;										\
	for ( size_t index = 0; __string[index] != '\0'; index++ ) {		\
		__VA_ARGS__														\
	}																	\
}while( 0 )

enum {
	StringDefaultCapacity 	= 100,
	StringGrowRate			= 2
};

//---------------------------------------------------------------------------------

WString*
__String( WString input )
{
	assert( input.capacity > 0 );

	if ( not input.cstring ) input.cstring = "";

	WString *self = __wxnew( WString,
		.size = utf8len( input.cstring ),
		.sizeBytes = strlen( input.cstring ) + 1,
		.capacity = input.capacity
	);

	self->capacity = __wmax( self->capacity, self->sizeBytes );

	//Copy the given char* into our new String*
	self->cstring = __wxmalloc( self->capacity );
	strcpy( self->cstring, input.cstring ),

	//Always terminate the cstring
	self->cstring[self->capacity - 1] = '\0';

	assert( self );
	return checkString( self );
}

WString*
wstring_new( const char* cstring, size_t capacity )
{
	assert( cstring );

	WString *self = __wxnew( WString,
		.size = utf8len( cstring ),
		.sizeBytes = strlen( cstring ) + 1,
		.capacity = capacity ? capacity : StringDefaultCapacity,
	);

	self->capacity = __wmax( self->capacity, self->sizeBytes );

	//Copy the given char* into our new String*
	self->cstring = __wxmalloc( self->capacity );
	strcpy( self->cstring, cstring ),

	//Always terminate the cstring
	self->cstring[self->capacity - 1] = '\0';

	assert( self );
	return checkString( self );
}

WString*
wstring_dup( const char* cstring )
{
	return wstring_new( cstring, strlen( cstring )+1 );
}

WString*
wstring_clone( const WString *self )
{
	assert( self != NULL );

	WString* clone = __String( (WString) {
			.capacity = self->capacity,
			.cstring = self->cstring
		}
	);

	assert( clone );
	assert( wstring_equals( clone, self ) );
	return checkString( clone );
}

void
wstring_clear( WString *self )
{
	assert( self );

	self->cstring[0] = '\0';
	self->size = 0;
	self->sizeBytes = 1;

	assert( wstring_empty( self ) );
	checkString( self );
}

char*
wstring_steal( WString **selfPointer )
{
	assert( selfPointer );
	assert( *selfPointer );

	WString *self = *selfPointer;

	char* stolen = self->cstring;
	self->cstring = NULL;
	wstring_delete( selfPointer );

	assert( stolen );
	return stolen;
}

char*
wstring_cstring( const WString* string )
{
	assert( string );

	return strdup( string->cstring );
}

void
wstring_delete( WString **selfPointer )
{
	if ( selfPointer == NULL or *selfPointer == NULL )
		return;

	WString *self = *selfPointer;

	free( self->cstring );
	free( *selfPointer );
	*selfPointer = NULL;
}

void
wstring_assign( WString** selfPointer, WString* other )
{
	if ( not selfPointer ) return;

	wstring_delete( selfPointer );
	*selfPointer = other;
}

//---------------------------------------------------------------------------------

WString*
wstring_append( WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	size_t newSize = self->sizeBytes + other->sizeBytes - 1;
	resize( self, newSize );

	strcpy( &self->cstring[self->sizeBytes - 1], other->cstring );
	self->size += other->size;
	self->sizeBytes += other->sizeBytes - 1;

	assert( self );
	assert( wstring_endsWith( self, other ));
	return checkString( self );
}

WString*
wstring_appendc( WString *self, const char* other )
{
	assert( self );
	assert( other );

	size_t sizeOther = strlen( other );
	size_t newSize = self->sizeBytes + sizeOther;
	resize( self, newSize );

	strcpy( &self->cstring[self->sizeBytes - 1], other );
	self->size += utf8len( other );
	self->sizeBytes += sizeOther;

	assert( self );
	return checkString( self );
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
wstring_appendf( WString* self, const char* format, ... )
{
	assert( self );
	assert( format );

	char* other;

    va_list args;
    va_start( args, format );
	if ( vasprintf( &other, format, args ) < 0 )
		__wdie( "vasprintf error." );
	va_end( args );

	wstring_appendc( self, other );

	free( other );
	assert( self );
    return checkString( self );
}

WString*
wstring_printf( const char* format, ... )
{
	assert( format );

	char* cstring;

    va_list args;
    va_start( args, format );
	if ( vasprintf( &cstring, format, args ) < 0 )
		__wdie( "vasprintf error." );
	va_end( args );

	WString* string = wstring_dup( cstring );

	free( cstring );
	assert( string );
	return checkString( string );
}

WString*
wstring_prepend( WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	size_t newSize = self->sizeBytes + other->sizeBytes - 1;
	resize( self, newSize );

	memmove( &self->cstring[other->sizeBytes - 1], self->cstring, self->sizeBytes );
	memmove( self->cstring, other->cstring, other->sizeBytes - 1 );

	self->size += other->size;
	self->sizeBytes += other->sizeBytes - 1;

	assert( self );
	assert( wstring_startsWith( self, other ) );
	return checkString( self );
}

//REFACTOR: wstring_replace() must use resize()
static WString*
_replace( WString *self, const char *search, const char *replace, bool all )
{
	assert( self );
	assert( search );
	assert( replace );

	size_t count = occurrences( self->cstring, search );
	if ( count == 0 ) return checkString( self );

	size_t searchLen = strlen( search );
	size_t replaceLen = strlen( replace );

	size_t newSizeBytes = (
        self->sizeBytes - 1
      - searchLen * count
      + replaceLen * count
	) + 1;

	size_t newCapacity = __wmax( self->capacity, newSizeBytes );
	char *newCString = __wxcalloc( newCapacity, 1 );

	char *pos = self->cstring;
	char *current;
	bool repeat = true;
	while (( current = strstr( pos, search )) and repeat ) {
		size_t len = current - pos;
		strncat( newCString, pos, len );
		strncat( newCString, replace, replaceLen );
		pos = current + searchLen;
		repeat = all;
	}

	if ( pos != ( self->cstring + self->sizeBytes - 1 ))
		strncat( newCString, pos, ( self->cstring - pos ));

	free( self->cstring );
	self->cstring = newCString;
	self->sizeBytes = strlen( self->cstring ) + 1;
	self->size = utf8len( self->cstring );
	self->capacity = newCapacity;
	self->cstring[newCapacity - 1] = 0;

	assert( self != NULL );
	return checkString( self );
}

WString*
wstring_replace( WString *self, const char* search, const char* replace )
{
	return _replace( self, search, replace, false );
}

WString*
wstring_replaceAll( WString *self, const char* search, const char* replace )
{
	return _replace( self, search, replace, true );
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
wstring_trim( WString *self, const char chars[] )
{
	wstring_ltrim( self, chars );
	wstring_rtrim( self, chars );

	return self;
}

WString*
wstring_ltrim( WString *self, const char chars[] )
{
	assert( self );
	assert( chars );
	assert( chars[0] );

	char *newString = self->cstring;
	size_t newLen = self->sizeBytes;

	while ( isTrimmable( *newString, chars ) ) {
		newString++;
		newLen--;
		self->size--;
		self->sizeBytes--;
	}

	if ( newLen > 0 )
		memmove( self->cstring, newString, newLen );

	assert( self != NULL );
	return checkString( self );
}

WString*
wstring_rtrim( WString *self, const char chars[] )
{
	assert( self );
	assert( chars );
	assert( chars[0] );

	if ( self->sizeBytes < 2 )
		return self;

	char *newEnd = &self->cstring[self->sizeBytes - 2];

	while ( newEnd > self->cstring and isTrimmable( *newEnd, chars ) ) {
		newEnd--;
		self->size--;
		self->sizeBytes--;
	}

	newEnd[1] = '\0';

	assert( self != NULL );
	return checkString( self );
}

WString*
wstring_squeeze( WString *self )
{
	assert( self != NULL );

    if ( self->sizeBytes <= 1 )
		return checkString( self );

    char *from = self->cstring + 1;
    char *to = self->cstring + 1;

	while ( *from ) {
		char byte = *from;
		if ( isspace( byte ) and byte == *( from - 1 ) ) {
			self->size--;
			self->sizeBytes--;
		}
		else
			*to++ = byte;

		from++;
	}

	*to = 0;

	assert( self != NULL );
	return checkString( self );
}

WString*
wstring_truncate( WString *self, size_t size )
{
	assert( self != NULL );

	if ( size >= self->size )
		return checkString( self );

	size_t charIndex = 1;
	wstring_foreachIndex( self, byteIndex,
		if ( charIndex > size ) {
			self->cstring[byteIndex] = 0;
			self->size = size;
			self->sizeBytes = byteIndex + 1;
			return checkString( self );
		}
		charIndex++;
	);

	assert( self != NULL );
	assert( wstring_size( self ) <= size );
	return checkString( self );
}

WString*
wstring_center( WString *self, size_t size )
{
	assert( self != NULL );
	assert( size > 0 );

	if ( size <= self->size )
		return checkString( self );

	resize( self, size );

	size_t numberSpacesLeft = ( size - self->size ) / 2;
	size_t numberSpacesRight = size - self->size - numberSpacesLeft;

	//Move the old string and fill up with spaces
	memmove( &self->cstring[numberSpacesLeft], self->cstring, self->sizeBytes );
	memset( self->cstring, ' ', numberSpacesLeft );
	memset( &self->cstring[self->sizeBytes + numberSpacesLeft - 1], ' ', numberSpacesRight );

	self->size = size;
	self->sizeBytes += ( numberSpacesLeft + numberSpacesRight );

	self->cstring[self->sizeBytes - 1] = 0;

	assert( self != NULL );
	assert( wstring_size( self ) >= size );
	return checkString( self );
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

size_t
wstring_size( const WString *self )
{
	assert( self != NULL );

	return self->size;
}

size_t
wstring_sizeBytes( const WString *self )
{
	assert( self != NULL );

	return self->sizeBytes;
}

bool
wstring_empty( const WString *self )
{
	assert( self != NULL );

	return self->size == 0;
}

bool
wstring_equals( const WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	return self->size == other->size and					//Faster comparison for unequal strings
		   self->sizeBytes == other->sizeBytes and			//Faster comparison for unequal strings
		   strcmp( self->cstring, other->cstring ) == 0;
}

int
wstring_compare( const WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	return strcmp( self->cstring, other->cstring );
}

//TODO: Only ASCII characters are compared correctly.
int
wstring_compareCase( const WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	return strcasecmp( self->cstring, other->cstring );
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
wstring_contains( const WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	return strstr( self->cstring, other->cstring ) != NULL;
}

bool
wstring_startsWith( const WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	return strstr( self->cstring, other->cstring ) == self->cstring;
}

bool
wstring_endsWith( const WString *self, const WString *other )
{
	assert( self != NULL );
	assert( other != NULL );

	if ( self->sizeBytes < other->sizeBytes )
		return false;

	size_t endPosition = self->sizeBytes - other->sizeBytes;

	return strcmp( &self->cstring[endPosition], other->cstring ) == 0;
}

//---------------------------------------------------------------------------------

void
wstring_split( const WString *self, const char *delimiters, void foreach( const WString* ))
{
	assert( self );
	assert( delimiters and delimiters[0] );
	assert( foreach );

	//Get the first substring.
	char* copy = strdup( self->cstring );
	char* strtokPtr = NULL;
	char* token = copy[0] ? strtok_r( copy, delimiters, &strtokPtr ) : "";

	while ( token ) {
		WString* tokenString = wstring_dup( token );
			foreach( tokenString );
        wstring_delete( &tokenString );
		token = strtok_r( NULL, delimiters, &strtokPtr );
	}

	free( copy );
}

//---------------------------------------------------------------------------------

int
wstring_toInt( const WString *self )
{
	assert( self );

	char *endptr;

	errno = 0;
	long value = strtol( self->cstring, &endptr, 10 );

	if (( errno == ERANGE and ( value == LONG_MAX or value == LONG_MIN )) or ( errno != 0 and value == 0 ))
		return INT_MIN;

	if ( endptr == self->cstring )
		return INT_MIN;

	return value;
}


double
wstring_toDouble( const WString *self )
{
	assert( self );

	char *endptr;

	errno = 0;
	double value = strtod( self->cstring, &endptr );

	if (( errno == ERANGE and ( value == DBL_MAX or value == DBL_MIN )) or ( errno != 0 and value == 0.0 ))
		return NAN;

	if ( endptr == self->cstring )
		return NAN;

	return value;
}
//---------------------------------------------------------------------------------

//Resize the string to the given new capacity.
static void
resize( WString *self, size_t newCapacity )
{
	if ( newCapacity > self->capacity ) {
		self->capacity = __wmax( newCapacity, self->capacity * WStringGrowthRate );
		self->cstring = __wxrealloc( self->cstring, self->capacity );
	}
	assert( self->capacity >= newCapacity );
//	checkString( self );
}

//Taken from Github, UTF8.h (Public Domain), then modified
size_t
utf8len( const char *str )
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
occurrences( const char *self, const char *search )
{
	assert( self );
	assert( search );

	const char *selfPosition = self;
	size_t count = 0;
	size_t searchLength = strlen( search );

	while ( ( selfPosition = strstr( selfPosition, search ) ) ) {
		selfPosition += searchLength;
		count++;
	}

	return count;
}

//---------------------------------------------------------------------------------
