#pragma once

#include "Base/BaseTypes.h"
#include "Math/MathHelpers.h"
#include "Debug/DVAssert.h"
#include <atomic>

namespace DAVA
{
//////////////////////////////////////////////////////////////////////////
//'pseudo-thread-safe' ring array. It's provide only thread-safe obtaining
// of reference to element. So, don't write to array in parallel with
// array iterating.
// It's made for performance reasons.
//////////////////////////////////////////////////////////////////////////

template <class T>
class ProfilerRingArray
{
public:
    ProfilerRingArray(uint32 _size)
    {
        DVASSERT(IsPowerOf2(_size) && "Size of RingArray should be pow of two");
        elementsCount = _size;
        mask = elementsCount - 1;
        elements = new T[elementsCount];
    }
    ~ProfilerRingArray()
    {
        SafeDeleteArray(elements);
    }
    ProfilerRingArray(const ProfilerRingArray& a)
    {
        elementsCount = a.elementsCount;
        elements = new T[elementsCount];
        memcpy(elements, a.elements, elementsCount * sizeof(T));
        mask = a.mask;
        head = a.head.load();
    }
    ProfilerRingArray& operator=(const ProfilerRingArray& a)
    {
        if (this != &a)
        {
            SafeDeleteArray(elements);
            new (this) ProfilerRingArray(a);
        }
        return (*this);
    }

    class iterator;
    class reverse_iterator;

    T& next()
    {
        return elements[head++ & mask];
    }
    iterator begin()
    {
        return iterator(elements, (head & mask), mask);
    }
    iterator end()
    {
        return iterator(elements, (head & mask) | (mask + 1), mask);
    }
    reverse_iterator rbegin()
    {
        return reverse_iterator(elements, ((head - 1) & mask) | (mask + 1), mask);
    }
    reverse_iterator rend()
    {
        return reverse_iterator(elements, (head - 1) & mask, mask);
    }
    size_t size() const
    {
        return elementsCount;
    }

private:
    class base_iterator
    {
    public:
        base_iterator() = default;
        base_iterator(T* data, uint32 _index, uint32 _mask)
            : arrayData(data)
            , index(_index)
            , mask(_mask)
        {
        }
        ~base_iterator() = default;

        bool operator==(const base_iterator& it) const
        {
            return (index == it.index) && (arrayData == it.arrayData);
        }
        bool operator!=(const base_iterator& it) const
        {
            return !(*this == it);
        }
        T& operator*() const
        {
            return arrayData[index & mask];
        }
        T* operator->() const
        {
            return &arrayData[index & mask];
        }

        T* arrayData = nullptr;
        uint32 index = 0;
        uint32 mask = 0;
    };

    class iterator final : public base_iterator
    {
    public:
        iterator(T* data, uint32 _index, uint32 _mask)
            : base_iterator(data, _index, _mask)
        {
        }
        iterator(const reverse_iterator& it)
        {
            this->arrayData = it.arrayData;
            this->mask = it.mask;
            this->index = it.index;
        }
        operator reverse_iterator() const
        {
            return reverse_iterator(this->arrayData, this->index, this->mask);
        }
        iterator operator+(uint32 n) const
        {
            iterator it(*this);
            it.index += n;
            return it;
        }
        iterator operator-(uint32 n) const
        {
            iterator it(*this);
            it.index -= n;
            return it;
        }
        iterator& operator++()
        {
            ++(this->index);
            return *this;
        }
        iterator operator++(int)
        {
            iterator prev = *this;
            ++(*this);
            return prev;
        }
        iterator& operator--()
        {
            --(this->index);
            return *this;
        }
        iterator operator--(int)
        {
            iterator prev = *this;
            --(*this);
            return prev;
        }
    };

    class reverse_iterator final : public base_iterator
    {
    public:
        reverse_iterator(T* data, uint32 _index, uint32 _mask)
            : base_iterator(data, _index, _mask)
        {
        }
        reverse_iterator(const iterator& it)
        {
            this->arrayData = it.arrayData;
            this->mask = it.mask;
            this->index = it.index;
        }
        operator iterator() const
        {
            return iterator(this->arrayData, this->index, this->mask);
        }
        reverse_iterator operator+(uint32 n) const
        {
            reverse_iterator it(*this);
            it.index -= n;
            return it;
        }
        reverse_iterator operator-(uint32 n) const
        {
            reverse_iterator it(*this);
            it.index += n;
            return it;
        }
        reverse_iterator& operator++()
        {
            --(this->index);
            return *this;
        }
        reverse_iterator operator++(int)
        {
            reverse_iterator prev = *this;
            ++(*this);
            return prev;
        }
        reverse_iterator& operator--()
        {
            ++(this->index);
            return *this;
        }
        reverse_iterator operator--(int)
        {
            reverse_iterator prev = *this;
            --(*this);
            return prev;
        }
    };

    T* elements = nullptr;
    uint32 elementsCount = 0;
    uint32 mask = 0;
    std::atomic<uint32> head = { 0 };
};

} // end namespace DAVA