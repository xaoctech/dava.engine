/*
	DAVA SDK
	Fast Names
	Author: Sergey Zdanevich
*/

#ifndef __DAVAENGINE_FAST_NAME__
#define __DAVAENGINE_FAST_NAME__

#include "Base/HashMap.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{

struct FastNameDB : public StaticSingleton<FastNameDB>
{
	FastNameDB()
		// namesHash init. size will be 4096 and default values for int will be -1
		: namesHash(HashMap<const char *, int>(4096, -1))
	{};

	~FastNameDB()
	{
		for(size_t i = 0; i < namesTable.size(); ++i)
		{
			if(NULL != namesTable[i])
			{
				free((void *) namesTable[i]);
				namesTable[i] = NULL;
			}
		}
	}

	Vector<const char *> namesTable;
	Vector<int> namesRefCounts;
	Vector<int> namesEmptyIndexes;
	HashMap<const char *, int> namesHash;
};

class FastName
{
public:
	FastName();
	FastName(const char *name);
	FastName(const FastName &_name);
	~FastName();

	const char* c_str() const;
	const char* operator*() const;
	FastName& operator=(const FastName &_name);
	bool operator==(const FastName &_name) const;
	bool operator!=(const FastName &_name) const;
	int Index() const;

private:
	int index;

#ifdef DAVA_DEBUG
	const char* debug_str_ptr;
#endif

	void AddRef(int i) const;
	void RemRef(int i) const;
};

};

#endif // __DAVAENGINE_FAST_NAME__
