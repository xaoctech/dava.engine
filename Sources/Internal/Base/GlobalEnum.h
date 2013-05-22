/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_GLOBAL_ENUM_H__
#define __DAVAENGINE_GLOBAL_ENUM_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

class EnumMap
{
public:
	EnumMap();
	~EnumMap();

	bool ToValue(const char *, int &e) const;
	const char* ToString(const int e) const;

	int GetCount() const;
	bool GetValue(size_t index, int &e) const;

	void Register(const int e, const char *) const;

protected:
	typedef DAVA::Map<int, DAVA::String> EnumMapContainer;
	mutable EnumMapContainer map;
};

template<typename T>
class GlobalEnumMap
{
public:
	explicit GlobalEnumMap();
	~GlobalEnumMap();

	static const EnumMap* Instance();

protected:
	static void RegisterAll();
	static void Register(const int e, const char *s);
};

EnumMap::EnumMap()
{}

EnumMap::~EnumMap()
{}

void EnumMap::Register(const int e, const char* s) const
{
	DVASSERT(!map.count(e));
	map[e] = DAVA::String(s);
}

bool EnumMap::ToValue(const char* s, int &e) const
{
	bool ret = false;
	EnumMapContainer::const_iterator i = map.begin();
	EnumMapContainer::const_iterator end = map.end();
	DAVA::String str(s);

	for(; i != end; ++i)
	{
		if(i->second == str)
		{
			e = i->first;
			ret = true;
			break;
		}
	}

	return ret;
}

const char* EnumMap::ToString(const int e) const
{
	const char* ret = NULL;

	if(map.count(e))
	{
		ret = map.at(e).c_str();
	}

	return ret;
}

int EnumMap::GetCount() const
{
	return map.size();
}

bool EnumMap::GetValue(size_t index, int &e) const
{
	bool ret = false;
	EnumMapContainer::const_iterator i = map.begin();

	if(index < map.size())
	{
		while(index > 0)
		{
			index--;
			i++;
		}

		if(i != map.end())
		{
			e = i->first;
			ret = true;
		}
	}

	return ret;
}

template<typename T>
const EnumMap* GlobalEnumMap<T>::Instance()
{
	static EnumMap enumMap;
	static bool initialized = false;

	if(!initialized)
	{
		initialized = true;
		RegisterAll();
	}

	return &enumMap;
}

template<typename T>
void GlobalEnumMap<T>::Register(const int e, const char *s)
{
	Instance()->Register(e, s);
}

#define ENUM_DECLARE(eType) template<> void GlobalEnumMap<eType>::RegisterAll()
#define ENUM_ADD(eValue) Register(eValue, #eValue)
#define ENUM_ADD_DESCR(eValue, eDescr) Register(eValue, eDescr)

// Usage:
//	ENUM_DECLARE(AnyEnumType)
//	{
//		ENUM_ADDS(AnyEnumType::Value1);
//		ENUM_ADD_DESCR(AnyEnumType::Value2, "Value2");
//	}

#endif // __DAVAENGINE_GLOBAL_ENUM_H__
