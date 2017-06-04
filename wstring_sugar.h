/**	@file
*/
/*  Copyright (c) 2017 Michael Nau
    This file is part of Workhorse String.

    Workhorse String is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Workhorse String is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Workhorse Array.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef WSTRING_SUGAR_H_INCLUDED
#define WSTRING_SUGAR_H_INCLUDED

#include "wstring.h"

//---------------------------------------------------------------------------------

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

	WString*	(*toLower)		(WString* string);
	WString*	(*toUpper)		(WString* string);
	WString*	(*toTitle)		(WString* string);

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
	.toLower = wstring_toLower,			\
	.toUpper= wstring_toUpper,			\
	.toTitle = wstring_toTitle,			\
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

#endif // WSTRING_SUGAR_H_INCLUDED
