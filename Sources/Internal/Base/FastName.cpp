/*
	DAVA SDK
	Fast Names
	Author: Sergey Zdanevich
*/

#include "FastName.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

FastName::FastName()
	: index(-1)
{
#ifdef DAVA_DEBUG
	debug_str_ptr = NULL;
#endif
}

FastName::FastName(const char *name)
	: index(-1)
{
	DVASSERT(NULL != name);
	FastNameDB *db = FastNameDB::Instance();

	// search if that name is already in hash
	if(db->namesHash.IsKey(name))
	{
		// already exist, so we just need to set the same index to this object
		index = db->namesHash[name];
		AddRef(index);
	}
	else
	{
		// string isn't in hash and it isn't in names table, so we need to copy it 
		// and find place for copied string in names table and set it
		size_t nameLen = strlen(name);
		char *nameCopy = (char *) malloc(nameLen + 1);
		memcpy(nameCopy, name, nameLen + 1);

		// search for empty indexes in names table
		if(db->namesEmptyIndexes.size() > 0)
		{
			// take last empty index from emptyIndexes table
			index = db->namesEmptyIndexes.back();
			db->namesEmptyIndexes.pop_back();
		}
		else
		{
			// index will be a new row in names table
			index = db->namesTable.size();
			db->namesTable.resize(index + 1);
			db->namesRefCounts.resize(index + 1);
		}

		// set name to names table
		db->namesTable[index] = nameCopy;
		db->namesRefCounts[index] = 1;

		// add name and its index into hash
		db->namesHash.Insert(nameCopy, index);
	}

	DVASSERT(index != -1);
	#ifdef DAVA_DEBUG
		debug_str_ptr = c_str();
	#endif
}

FastName::FastName(const FastName &_name)
{
	index = _name.index;

#ifdef DAVA_DEBUG
	debug_str_ptr = _name.debug_str_ptr;
#endif

	AddRef(index);
}

FastName::~FastName()
{
	RemRef(index);
}

const char* FastName::c_str() const
{
	DVASSERT(index >= -1 && index < (int)FastNameDB::Instance()->namesTable.size());
	if(index >= 0)
	{
        return FastNameDB::Instance()->namesTable[index];
	}
    
	return NULL;
}

FastName& FastName::operator=(const FastName &_name)
{
	RemRef(index);

	index = _name.index;

#ifdef DAVA_DEBUG
	debug_str_ptr = _name.debug_str_ptr;
#endif

	AddRef(index);
	return *this;
}

bool FastName::operator==(const FastName &_name) const
{
	return index == _name.index;
}

bool FastName::operator!=(const FastName &_name) const
{
	return index != _name.index;
}

const char* FastName::operator*() const
{
    return c_str();
}

int FastName::Index() const
{
	return index;
}

void FastName::AddRef(int i) const
{
	FastNameDB *db = FastNameDB::Instance();
	DVASSERT(i >= 0 && i < (int)db->namesTable.size());
	db->namesRefCounts[i]++;
}

void FastName::RemRef(int i) const
{
	FastNameDB *db = FastNameDB::Instance();
	DVASSERT(i >= -1 && i < (int)db->namesTable.size());
	if (i >= 0)
	{
		db->namesRefCounts[i]--;
		if(0 == db->namesRefCounts[i])
		{
			// remove name and index from hash
			db->namesHash.Remove(db->namesTable[i]);

			// delete allocated memory for this string
			free((void *) db->namesTable[i]);

			// remove name from names table
			db->namesTable[i] = NULL;

			// remember that this index is empty already
			db->namesEmptyIndexes.push_back(i);
		}
	}
}

};
