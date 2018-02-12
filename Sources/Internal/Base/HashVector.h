#pragma once

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
/**
Hybrid container which stores objects' pointers in both vector and hashmap. Does not keep order of elements after 'Remove' calls.
*/
template <class T>
class HashVector
{
    class Iterator
    {
        T** data;
        int32 size;
        int32 position;

    public:
        Iterator(T** data, int32 size, int32 position)
            : data(data)
            , size(size)
            , position(position)
        {
        }

        T* operator*()
        {
            DVASSERT(position < size);
            return data[position];
        }
        Iterator& operator++()
        {
            position++;
            return *this;
        }
        bool operator!=(const Iterator& it) const
        {
            return position != it.position;
        }
    };

public:
    /** Add specified 'value' to container. */
    void Add(T* value);

    /** Remove specified 'value' from container. The behavior is undefined unless 'value' is already added to container exactly once. Change order of 'GetObjects' elements. */
    void Remove(T* value);

    /** Check if specified value is present in container. */
    bool Contains(T* value);

    /** Return vector containing all objects added with 'Add'. */
    const Vector<T*>& GetObjects();

    /** Return object with specified 'index'. The behavior is undefined unless 'index' is less than 'GetSize()' value. */
    T* GetObjectAt(uint32 index);

    /** Return index of object with specified 'value'. The behavior is undefined unless 'value' is already added to container. */
    uint32 GetIndexOf(T* value);

    /** Return number of objects in container. */
    uint32 GetSize();

    /** Clear contents by removing all items. */
    void Clear();

    /** begin for range-based support */
    Iterator begin()
    {
        return { vector.data(), static_cast<int32>(vector.size()), 0 };
    }

    /** end for range-based support */
    Iterator end()
    {
        return { vector.data(), static_cast<int32>(vector.size()), static_cast<int32>(vector.size()) };
    }

private:
    Vector<T*> vector;
    UnorderedMap<T*, uint32> map;
};
}

#include "Base/Private/HashVector.hpp"
