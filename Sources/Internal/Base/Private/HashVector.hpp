#include "Debug/DVAssert.h"

namespace DAVA
{

template<class T>
const Vector<T*>& HashVector<T>::GetObjects()
{
    return vector;
}

template<class T>
uint32 HashVector<T>::GetSize()
{
    return static_cast<uint32>(vector.size());
}

template<class T>
void HashVector<T>::Add(T* value)
{
    DVASSERT(map.find(value) == map.end());
    vector.push_back(value);
    map[value] = static_cast<uint32>(vector.size() - 1);
}

template<class T>
void HashVector<T>::Remove(T * value)
{
    auto it = map.find(value);
    DVASSERT(it != map.end());

    uint32 index = it->second;

    if(vector.size() > 1)
    {
        //swap-delete from vector
        T*& last = vector.back();
        vector[index] = last;

        //update swapped in map
        auto lastIt = map.find(last);
        DVASSERT(lastIt != map.end());
        lastIt->second = index;
        
        vector.pop_back();
    }
    else
    {
        vector.clear();
    }

    //delete from map
    map.erase(it);
}


template<class T>
bool HashVector<T>::Contains(T* value)
{
    return map.find(value) != map.end();
}


template<class T>
T* HashVector<T>::GetObjectAt(uint32 index)
{
    DVASSERT(index < vector.size());
    return vector[index];
}

template<class T>
uint32 HashVector<T>::GetIndexOf(T* value)
{
    auto it = map.find(value);
    DVASSERT(it != map.end());
    return static_cast<uint32>(it->second);
}


template <class T>
void HashVector<T>::Clear()
{
    vector.clear();
    map.clear();
}

}
