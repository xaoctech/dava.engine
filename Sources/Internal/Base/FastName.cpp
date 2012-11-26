/*
	DAVA SDK
	Fast Names
	Author: Sergey Zdanevich
*/

#include "FastName.h"

namespace DAVA
{

// namesHash init. size will be 4096 and default values for int will be -1
//Vector<const char *> FastName::namesTable;
//Vector<int> FastName::namesRefCounts;
//Vector<int> FastName::namesEmptyIndexes;
//HashMap<const char *, int> FastName::namesHashs = ;

FastName::FastName()
	: index(-1)
{ }

FastName::FastName(const char *name)
	: index(-1)
{
	DVASSERT(NULL != name);

	// search if that name is already in hash
	if(FastNameData::Instance()->namesHashs.HasKey(name))
	{
		// already exist, so we just need to set the same index to this object
		index = FastNameData::Instance()->namesHashs[name];
		FastNameData::Instance()->namesRefCounts[index]++;
	}
	else
	{
		// string isn't in hash and it isn't in names table, so we need to copy it 
		// and find place for copied string in names table and set it
		size_t nameLen = strlen(name);
		char *nameCopy = new char[nameLen + 1];
		memcpy(nameCopy, name, nameLen + 1);

		// search for empty indexes in names table
		if(FastNameData::Instance()->namesEmptyIndexes.size() > 0)
		{
			// take last empty index from emptyIndexes table
			index = FastNameData::Instance()->namesEmptyIndexes.back();
			FastNameData::Instance()->namesEmptyIndexes.pop_back();
		}
		else
		{
			// index will be a new row in names table
			index = FastNameData::Instance()->namesTable.size();
			FastNameData::Instance()->namesTable.resize(index + 1);
			FastNameData::Instance()->namesRefCounts.resize(index + 1);
		}

		// set name to names table
		FastNameData::Instance()->namesTable[index] = nameCopy;
		FastNameData::Instance()->namesRefCounts[index] = 1;

		// add name and its index into hash
		FastNameData::Instance()->namesHashs.Insert(name, index);
	}
}

FastName::~FastName()
{
	FastNameData::Instance()->namesRefCounts[index]--;
	const char *tmp = FastNameData::Instance()->namesTable[index];

	if(0 == FastNameData::Instance()->namesRefCounts[index])
	{
		// remove name and index from hash
		FastNameData::Instance()->namesHashs.Remove(FastNameData::Instance()->namesTable[index]);

		// delete allocated memory for this string
		delete[] FastNameData::Instance()->namesTable[index];

		// remove name from names table
		FastNameData::Instance()->namesTable[index] = NULL;

		// remember that this index is empty already
		FastNameData::Instance()->namesEmptyIndexes.push_back(index);
	}
}

};
