#ifndef __DAVAENGINE_GLOBAL_ENUM_H__
#define __DAVAENGINE_GLOBAL_ENUM_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

template<typename T>
class GlobalEnum
{
public:
	explicit GlobalEnum();
	~GlobalEnum();

	static bool ToValue(const char *, int &e);
	static const char* ToString(const int e);

	static int GetCount();
	static bool GetValue(size_t index, int &e);

private:
	typedef DAVA::Map<int, DAVA::String> ETOSMap;
	static ETOSMap& GetMap();

	static void RegisterAll();
	static void Register(const int e, const char *);
};

template<typename T>
void GlobalEnum<T>::Register(const int e, const char* s)
{
	ETOSMap& etos = GetMap();

	DVASSERT(!etos.count(e));
	etos[e] = DAVA::String(s);
}

template<typename T>
bool GlobalEnum<T>::ToValue(const char* s, int &e)
{
	bool ret = false;
	ETOSMap &etos = GetMap();

	ETOSMap::iterator i = etos.begin();
	ETOSMap::iterator end = etos.end();
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

template<typename T>
const char* GlobalEnum<T>::ToString(const int e)
{
	const char* ret = NULL;
	ETOSMap &etos = GetMap();

	if(etos.count(e))
	{
		ret = etos[e].c_str();
	}

	return ret;
}

template<typename T>
int GlobalEnum<T>::GetCount()
{
	return GetMap().size();
}

template<typename T>
bool GlobalEnum<T>::GetValue(size_t index, int &e)
{
	bool ret = false;
	ETOSMap &etos = GetMap();
	ETOSMap::iterator i = etos.begin();

	if(index < etos.size())
	{
		while(index > 0)
		{
			index--;
			i++;
		}

		if(i != etos.end())
		{
			e = i->first;
			ret = true;
		}
	}

	return ret;
}

template<typename T>
typename GlobalEnum<T>::ETOSMap& GlobalEnum<T>::GetMap()
{
	static ETOSMap etosMap;
	static bool initialized = false;

	if(!initialized)
	{
		initialized = true;
		RegisterAll();
	}

	return etosMap;
}

#define ENUM_DECLARE(eType) template<> void GlobalEnum<eType>::RegisterAll()
#define ENUM_ADD(eValue) Register(eValue, #eValue)
#define ENUM_ADD_DESCR(eValue, eDescr) Register(eValue, eDescr)

// Usage:
//	ENUM_DECLARE(AnyEnumType)
//	{
//		ENUM_ADDS(AnyEnumType::Value1);
//		ENUM_ADD_DESCR(AnyEnumType::Value2, "Value2");
//	}

#endif // __DAVAENGINE_GLOBAL_ENUM_H__
