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

	bool Compare(const FastName &name1, const FastName &name2) const
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
    
    inline bool operator == (const FastNameSet & _another) const;

private:
	int Insert(const char *name, const int &value);
	int Value(const FastName &key) const;
	int operator()(const FastName &key) const;
};

template <typename V>
class FastNameMap : public HashMap<FastName, V>
{
public:
	void Insert(const char *name, const V &value)
	{
		HashMap<FastName, V>::Insert(FastName(name), value);
	}
	void Insert(const FastName & name, const V &value)
	{
		HashMap<FastName, V>::Insert(name, value);
	}
};
    
// Implementation
inline bool FastNameSet::operator == (const FastNameSet & _another) const
{
    if (Size() != _another.Size())return false;
    FastNameSet::Iterator it = this->Begin();
    const FastNameSet::Iterator & endIt = End();
    for (; it !=  endIt; ++it)
    {
        const FastName & name = it.GetKey();
        if (!_another.IsKey(name))return false;
    }
    return true;
}


};

#endif // __DAVAENGINE_FAST_NAME_MAP__
