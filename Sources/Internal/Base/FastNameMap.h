/*
	DAVA SDK
	Fast Names Map
	Author: Sergey Zdanevich
*/

#ifndef __DAVAENGINE_FAST_NAME_MAP__
#define __DAVAENGINE_FAST_NAME_MAP__

#include "FastName.h"

namespace DAVA
{

template<> struct Hash<FastName>
{
	size_t operator()(const FastName &name) const
	{
		return name.Index();
	}

	bool Compare(const FastName &name1, const FastName &name2)
	{
		return name1.Index() == name2.Index();
	}
};

class FastNameSet : public HashMap<FastName, int>
{
public:
	void Insert(const char *name)
	{
		HashMap<FastName, int>::Insert(FastName(name), 0);
	}

private:
	int Insert(const char *name, const int &value);
	int Value(const FastName &key);
	int operator()(const FastName &key);
};

template <typename V>
class FastNameMap : public HashMap<FastName, V>
{
public:
	void Insert(const char *name, const V &value)
	{
		HashMap<FastName, V>::Insert(FastName(name), value);
	}
};

};

#endif // __DAVAENGINE_FAST_NAME_MAP__
