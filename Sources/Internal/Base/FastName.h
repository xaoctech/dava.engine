/*
	DAVA SDK
	Fast Names
	Author: Sergey Zdanevich
*/

#ifndef __DAVAENGINE_FAST_NAME__
#define __DAVAENGINE_FAST_NAME__

#include "HashMap.h"
#include "Debug/DVAssert.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{

class FastNameData : public StaticSingleton<FastNameData>
{
public:
	Vector<const char *> namesTable;
	Vector<int> namesRefCounts;
	Vector<int> namesEmptyIndexes;
	HashMap<const char *, int> namesHashs;

	FastNameData()
		: namesHashs(HashMap<const char *, int>(4096, -1))
	{ }
};

class FastName
{
private:

public:
	FastName();
	FastName(const char *name);
	~FastName();

	FastName(const FastName &_name)
	{
		index = _name.index;
		FastNameData::Instance()->namesRefCounts[index]++;
	}

	FastName& operator=(const FastName &_name)
	{
		index = _name.index;
		FastNameData::Instance()->namesRefCounts[index]++;
		return *this;
	}

	const char* operator*() const
	{
		DVASSERT((size_t) index < FastNameData::Instance()->namesTable.size());
		DVASSERT(NULL != FastNameData::Instance()->namesTable[index]);
		return FastNameData::Instance()->namesTable[index];
	}

	bool operator==(const FastName &_name) const
	{
		return index == _name.index;
	}

	bool operator!=(const FastName &_name) const
	{
		return index != _name.index;
	}

	int Index() const
	{
		return index;
	}

private:
	int index;

};

};

#endif // __DAVAENGINE_FAST_NAME__
