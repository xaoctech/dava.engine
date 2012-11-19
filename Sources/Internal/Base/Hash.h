/*
    DAVA SDK
    Hash 
    Author: Sergey Zdanevich
 */

#ifndef __DAVAENGINE_HASH__
#define __DAVAENGINE_HASH__

#include <stddef.h>
#include "Base/BaseTypes.h"

namespace DAVA
{
	// default hash function for strings
	inline size_t DavaHashString(const char* str)
	{
		size_t hash = 0;
		for (; *str; ++str)
		{
			hash = 5 * hash + *str;
		}
		return hash;
	}

	// Base hash type
	// Any child (template specialized) structure
	// should implement for specific type T:
	// - hash function: operator()(T value)
	// - compare function: compare(T value1, T value2)
	template <typename T>
	struct Hash
	{ };

	// specialization for char *
	template<> struct Hash <char *>
	{
		size_t operator()(const char *str) const
		{
			return DavaHashString(str);
		}

		bool Compare(const char *str1, const char *str2)
		{
			return (str1 == str2) || (0 == strcmp(str1, str2));
		}
	};

	// specialization for const char *
	template<> struct Hash <const char *>
	{
		size_t operator()(const char *str) const
		{
			return DavaHashString(str);
		}

		bool Compare(const char *str1, const char *str2)
		{
			return (str1 == str2) || (0 == strcmp(str1, str2));
		}
	};

	// specialization for const DAVA::String &
	template<> struct Hash <DAVA::String>
	{
		size_t operator()(const DAVA::String &str) const
		{
			return DavaHashString(str.c_str());
		}

		bool Compare(const DAVA::String &str1, const DAVA::String &str2)
		{
			return (str1 == str2);
		}
	};

	// specialization for DAVA::String *
	template<> struct Hash <DAVA::String *>
	{
		size_t operator()(const DAVA::String *str) const
		{
			return DavaHashString(str->c_str());
		}

		bool Compare(const DAVA::String *str1, const DAVA::String *str2)
		{
			return (str1 == str2) || 0 == str1->compare(str2->c_str());
		}
	};

	// specialization for const DAVA::String *
	template<> struct Hash <const DAVA::String *>
	{
		size_t operator()(const DAVA::String *str) const
		{
			return DavaHashString(str->c_str());
		}

		bool Compare(const DAVA::String *str1, const DAVA::String *str2)
		{
			return (str1 == str2) || 0 == str1->compare(str2->c_str());
		}
	};
};

#endif // __DAVAENGINE_HASH__
