#pragma once

#include <Base/Any.h>
#include <Debug/DVAssert.h>

#include <initializer_list>
#include <utility>

namespace DAVA
{
template <typename T>
class FixedVector
{
public:
    using iterator = T*;
    using const_iterator = const T*;

    FixedVector(std::size_t maxSize);
    FixedVector(std::size_t maxSize, std::size_t initCount, const T& value = T());
    FixedVector(std::size_t maxSize, std::initializer_list<T> init);
    FixedVector(const FixedVector& other);
    FixedVector& operator=(const FixedVector& other);
    FixedVector(FixedVector&& other);
    FixedVector& operator=(FixedVector&& other);
    ~FixedVector();

    T& operator[](std::size_t pos);
    const T& operator[](std::size_t pos) const;

    T& front();
    const T& front() const;
    T& back();
    const T& back() const;

    T* data() noexcept;
    const T* data() const noexcept;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;

    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    bool empty() const noexcept;
    std::size_t size() const noexcept;
    std::size_t max_size() const noexcept;
    std::size_t capacity() const noexcept;

    void clear() noexcept;
    void push_back(const T& value);
    void push_back(T&& value);
    template <typename... Args>
    T& emplace_back(Args&&... args);
    void pop_back();
    void resize(std::size_t count);
    void resize(std::size_t count, const T& value);

private:
    T* elemBegin = nullptr; // pointer to array beginning
    T* elemLast = nullptr; // pointer to current end
    T* elemEnd = nullptr; // pointer to array end
};

////  FixedVector implementation  ////////////////////////////////////////

template <typename T>
FixedVector<T>::FixedVector(std::size_t maxSize)
    : elemBegin(static_cast<T*>(::operator new(maxSize * sizeof(T))))
    , elemLast(elemBegin)
    , elemEnd(elemBegin + maxSize)
{
    DVASSERT(maxSize > 0);
}

template <typename T>
FixedVector<T>::FixedVector(std::size_t maxSize, std::size_t initCount, const T& value)
    : elemBegin(static_cast<T*>(::operator new(maxSize * sizeof(T))))
    , elemLast(elemBegin + initCount)
    , elemEnd(elemBegin + maxSize)
{
    DVASSERT(maxSize > 0 && initCount <= maxSize);
    for (std::size_t i = 0; i < initCount; ++i)
    {
        new (&elemBegin[i]) T(value);
    }
}

template <typename T>
FixedVector<T>::FixedVector(std::size_t maxSize, std::initializer_list<T> init)
    : elemBegin(static_cast<T*>(::operator new(maxSize * sizeof(T))))
    , elemLast(elemBegin + init.size())
    , elemEnd(elemBegin + maxSize)
{
    DVASSERT(maxSize > 0 && init.size() <= maxSize);
    std::size_t i = 0;
    for (const T& v : init)
    {
        new (&elemBegin[i]) T(v);
        i += 1;
    }
}

template <typename T>
FixedVector<T>::FixedVector(const FixedVector& other)
    : elemBegin(static_cast<T*>(::operator new(other.max_size() * sizeof(T))))
    , elemLast(elemBegin + other.size())
    , elemEnd(elemBegin + other.max_size())
{
    for (std::size_t i = 0, n = size(); i < n; ++i)
    {
        new (&elemBegin[i]) T(other.elemBegin[i]);
    }
}

template <typename T>
FixedVector<T>& FixedVector<T>::operator=(const FixedVector& other)
{
    DVASSERT(max_size() == other.max_size());
    if (this != &other)
    {
        clear();
        elemLast = elemBegin + other.size();
        for (std::size_t i = 0, n = size(); i < n; ++i)
        {
            new (&elemBegin[i]) T(other.elemBegin[i]);
        }
    }
    return *this;
}

template <typename T>
FixedVector<T>::FixedVector(FixedVector&& other)
    : elemBegin(static_cast<T*>(::operator new(other.max_size() * sizeof(T))))
    , elemLast(elemBegin + other.size())
    , elemEnd(elemBegin + other.max_size())
{
    for (std::size_t i = 0, n = size(); i < n; ++i)
    {
        new (&elemBegin[i]) T(std::move(other.elemBegin[i]));
    }
    other.clear();
}

template <typename T>
FixedVector<T>& FixedVector<T>::operator=(FixedVector&& other)
{
    DVASSERT(max_size() == other.max_size());
    if (this != &other)
    {
        clear();
        elemLast = elemBegin + other.size();
        for (size_t i = 0, n = other.size(); i < n; ++i)
        {
            elemBegin[i] = std::move(other.elemBegin[i]);
        }
        other.clear();
    }
    return *this;
}

template <typename T>
FixedVector<T>::~FixedVector()
{
    clear();
    ::operator delete(elemBegin);
    elemBegin = nullptr;
    elemLast = nullptr;
    elemEnd = nullptr;
}

template <typename T>
T& FixedVector<T>::operator[](std::size_t pos)
{
    DVASSERT(pos < size());
    return elemBegin[pos];
}

template <typename T>
const T& FixedVector<T>::operator[](std::size_t pos) const
{
    DVASSERT(pos < size());
    return elemBegin[pos];
}

template <typename T>
T& FixedVector<T>::front()
{
    DVASSERT(size() > 0);
    return elemBegin[0];
}

template <typename T>
const T& FixedVector<T>::front() const
{
    DVASSERT(size() > 0);
    return elemBegin[0];
}

template <typename T>
T& FixedVector<T>::back()
{
    DVASSERT(size() > 0);
    return elemLast[-1];
}

template <typename T>
const T& FixedVector<T>::back() const
{
    DVASSERT(size() > 0);
    return elemLast[-1];
}

template <typename T>
T* FixedVector<T>::data() noexcept
{
    return elemBegin;
}

template <typename T>
const T* FixedVector<T>::data() const noexcept
{
    return elemBegin;
}

template <typename T>
typename FixedVector<T>::iterator FixedVector<T>::begin() noexcept
{
    return iterator(elemBegin);
}

template <typename T>
typename FixedVector<T>::const_iterator FixedVector<T>::begin() const noexcept
{
    return const_iterator(elemBegin);
}

template <typename T>
typename FixedVector<T>::const_iterator FixedVector<T>::cbegin() const noexcept
{
    return const_iterator(elemBegin);
}

template <typename T>
typename FixedVector<T>::iterator FixedVector<T>::end() noexcept
{
    return iterator(elemLast);
}

template <typename T>
typename FixedVector<T>::const_iterator FixedVector<T>::end() const noexcept
{
    return const_iterator(elemLast);
}

template <typename T>
typename FixedVector<T>::const_iterator FixedVector<T>::cend() const noexcept
{
    return const_iterator(elemLast);
}

template <typename T>
void FixedVector<T>::clear() noexcept
{
    for (std::size_t i = 0, n = size(); i < n; ++i)
    {
        elemBegin[i].~T();
    }
    elemLast = elemBegin;
}

template <typename T>
void FixedVector<T>::push_back(const T& value)
{
    DVASSERT(size() < max_size());
    new (elemLast) T(value);
    elemLast += 1;
}

template <typename T>
void FixedVector<T>::push_back(T&& value)
{
    DVASSERT(size() < max_size());
    new (elemLast) T(std::move(value));
    elemLast += 1;
}

template <typename T>
template <typename... Args>
T& FixedVector<T>::emplace_back(Args&&... args)
{
    DVASSERT(size() < max_size());
    new (elemLast) T(std::forward<Args>(args)...);
    elemLast += 1;
    return back();
}

template <typename T>
void FixedVector<T>::pop_back()
{
    DVASSERT(size() > 0);
    elemLast -= 1;
    elemLast->~T();
}

template <typename T>
void FixedVector<T>::resize(std::size_t count)
{
    DVASSERT(count <= max_size());
    const std::size_t curSize = size();
    if (count > curSize)
    {
        for (std::size_t i = curSize; i < count; ++i)
        {
            new (&elemBegin[i]) T();
        }
        elemLast = elemBegin + count;
    }
    else if (count < curSize)
    {
        for (std::size_t i = curSize; i-- > count;)
        {
            elemBegin[i].~T();
        }
        elemLast = elemBegin + count;
    }
}

template <typename T>
void FixedVector<T>::resize(std::size_t count, const T& value)
{
    DVASSERT(count <= max_size());
    const std::size_t curSize = size();
    if (count > curSize)
    {
        for (std::size_t i = curSize; i < count; ++i)
        {
            new (&elemBegin[i]) T(value);
        }
        elemLast = elemBegin + count;
    }
    else if (count < curSize)
    {
        for (std::size_t i = curSize; i-- > count;)
        {
            elemBegin[i].~T();
        }
        elemLast = elemBegin + count;
    }
}

template <typename T>
bool FixedVector<T>::empty() const noexcept
{
    return elemBegin == elemLast;
}

template <typename T>
std::size_t FixedVector<T>::size() const noexcept
{
    return elemLast - elemBegin;
}

template <typename T>
std::size_t FixedVector<T>::max_size() const noexcept
{
    return elemEnd - elemBegin;
}

template <typename T>
std::size_t FixedVector<T>::capacity() const noexcept
{
    return max_size();
}

template <typename U>
bool operator==(const FixedVector<U>& v1, const FixedVector<U>& v2)
{
    if (v1.max_size() == v2.max_size() && v1.size() == v2.size())
    {
        auto compare = [](const FixedVector<U>& v1, const FixedVector<U>& v2) -> bool {
            auto it1 = v1.cbegin();
            auto e1 = v1.cend();
            for (auto it2 = v2.cbegin(); it1 != e1 && *it1 == *it2; ++it1, ++it2)
            {
            }
            return it1 == e1;
        };
        return compare(v1, v2);
    }
    return false;
}

template <typename U>
bool operator!=(const FixedVector<U>& v1, const FixedVector<U>& v2)
{
    return !(v1 == v2);
}

template <typename T>
struct AnyCompare<FixedVector<T>>
{
    static bool IsEqual(const Any& any1, const Any& any2)
    {
        const FixedVector<T>& v1 = any1.Get<FixedVector<T>>();
        const FixedVector<T>& v2 = any2.Get<FixedVector<T>>();
        return v1 == v2;
    }
};

} // namespace DAVA
