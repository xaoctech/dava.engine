/*
	DAVA SDK
	Fast Names
	Author: Sergey Zdanevich
*/

#include "FastName.h"

namespace DAVA
{

// namesHash init. size will be 4096 and default values for int will be -1
Vector<const char *> FastName::namesTable;
Vector<int> FastName::namesRefCounts;
Vector<int> FastName::namesEmptyIndexes;
HashMap<const char *, int> FastName::namesHash = HashMap<const char *, int>(4096, -1);

FastName::FastName()
	: index(-1)
{ }

FastName::FastName(const char *name)
	: index(-1)
{
	DVASSERT(NULL != name);

	// search if that name is already in hash
	if(namesHash.HasKey(name))
	{
		// already exist, so we just need to set the same index to this object
		index = namesHash[name];
		namesRefCounts[index]++;
	}
	else
	{
		// string isn't in hash and it isn't in names table, so we need to copy it 
		// and find place for copied string in names table and set it
		size_t nameLen = strlen(name);
		char *nameCopy = new char[nameLen + 1];
		memcpy(nameCopy, name, nameLen + 1);

		// search for empty indexes in names table
		if(namesEmptyIndexes.size() > 0)
		{
			// take last emty index from emptyIndexes table
			index = namesEmptyIndexes.back();
			namesEmptyIndexes.pop_back();
		}
		else
		{
			// index will be a new row in names table
			index = namesTable.size();
			namesTable.resize(index + 1);
			namesRefCounts.resize(index + 1);
		}

		// set name to names table
		namesTable[index] = nameCopy;
		namesRefCounts[index] = 1;

		// add name and its index into hash
		namesHash.Insert(name, index);
	}
}

FastName::~FastName()
{
	namesRefCounts[index]--;
	const char *tmp = namesTable[index];

	if(0 == namesRefCounts[index])
	{
		printf("111\n");
		/*
		// remove name and index from hash
		namesHash.Remove(namesTable[index]);

		// delete allocated memory for this string
		delete[] namesTable[index];

		// remove name from names table
		namesTable[index] = NULL;

		// remember that this index is empty already
		namesEmptyIndexes.push_back(index);
		*/
	}
}

};
