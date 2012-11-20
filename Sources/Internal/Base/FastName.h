/*
	DAVA SDK
	Fast Names
	Author: Sergey Zdanevich
*/

#ifndef __DAVAENGINE_FAST_NAME__
#define __DAVAENGINE_FAST_NAME__

#include "HashMap.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

class FastName
{
private:
	static Vector<const char *> namesTable;
	static Vector<int> namesRefCounts;
	static Vector<int> namesEmptyIndexes;
	static HashMap<const char *, int> namesHash;

public:
	FastName();
	FastName(const char *name);
	~FastName();

	FastName(const FastName &_name)
	{
		index = _name.index;
		namesRefCounts[index]++;
	}

	FastName& operator=(const FastName &_name)
	{
		index = _name.index;
		namesRefCounts[index]++;
		return *this;
	}

	const char* operator*() const
	{
		DVASSERT((size_t) index < namesTable.size());
		DVASSERT(NULL != namesTable[index]);
		return namesTable[index];
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
