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
