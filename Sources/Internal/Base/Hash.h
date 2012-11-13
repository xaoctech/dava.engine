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
	inline size_t dava_hash_string(const char* str)
	{
		size_t hash = 0; 
		for (; *str; ++str)
		{
			hash = 5 * hash + *str;
		}
		return hash;
	}

	template <typename T>
	struct Hash
	{ };

	template<> struct Hash <char *>
	{
		size_t operator()(const char *str) const
		{
			return dava_hash_string(str);
		}

		bool compare(const char *str1, const char *str2)
		{
			return (str1 == str2) || (0 == strcmp(str1, str2));
		}
	};

	template<> struct Hash <const char *>
	{
		size_t operator()(const char *str) const
		{
			return dava_hash_string(str);
		}

		bool compare(const char *str1, const char *str2)
		{
			return (str1 == str2) || (0 == strcmp(str1, str2));
		}
	};

	template<> struct Hash <DAVA::String>
	{
		size_t operator()(const DAVA::String &str) const
		{
			return dava_hash_string(str.c_str());
		}

		bool compare(const DAVA::String &str1, const DAVA::String &str2)
		{
			return (str1 == str2);
		}
	};

	template<> struct Hash <DAVA::String *>
	{
		size_t operator()(const DAVA::String *str) const
		{
			return dava_hash_string(str->c_str());
		}

		bool compare(const DAVA::String *str1, const DAVA::String *str2)
		{
			return (str1 == str2) || 0 == str1->compare(str2->c_str());
		}
	};

	template<> struct Hash <const DAVA::String *>
	{
		size_t operator()(const DAVA::String *str) const
		{
			return dava_hash_string(str->c_str());
		}

		bool compare(const DAVA::String *str1, const DAVA::String *str2)
		{
			return (str1 == str2) || 0 == str1->compare(str2->c_str());
		}
	};
};

#endif // __DAVAENGINE_HASH__
