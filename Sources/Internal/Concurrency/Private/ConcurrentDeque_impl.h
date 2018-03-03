#pragma once
#ifndef __DAVA_CONCURRENCY_DEQUE__
#include "Concurrency/ConcurrentDeque.h"
#endif

namespace DAVA
{
template <typename T>
ConcurrentDeque<T>::ConcurrentDeque()
    : isEmpty(true)
    , canceled(false)
{
}

template <typename T>
void ConcurrentDeque<T>::ProcessDeque(const Function<void(Deque<T>&)>& fn)
{
    UniqueLock<Mutex> guard(mutex);

    bool hadElements = !deque.empty();

    fn(deque);

    bool hasElements = !deque.empty();

    isEmpty = !hasElements;

    if (hadElements == false && hasElements == true)
    {
        conditionVar.NotifyAll();
    }
}

template <typename T>
void ConcurrentDeque<T>::PushBack(T const& t)
{
    UniqueLock<Mutex> guard(mutex);

    bool doSignal = deque.empty();

    deque.push_back(t);
    isEmpty = false;

    if (doSignal == true)
    {
        conditionVar.NotifyAll();
    }
}

template <typename T>
void ConcurrentDeque<T>::PushFront(T const& t)
{
    UniqueLock<Mutex> guard(mutex);

    bool doSignal = deque.empty();

    deque.push_front(t);
    isEmpty = false;

    if (doSignal == true)
    {
        conditionVar.NotifyAll();
    }
}

template <typename T>
bool ConcurrentDeque<T>::WaitNonEmpty(UniqueLock<Mutex>& lock)
{
    while ((isEmpty = deque.empty()))
    {
        if (canceled == true)
            break;

        conditionVar.Wait(lock);
    }

    if (canceled == true)
        return true;

    return false;
}

template <typename T>
T ConcurrentDeque<T>::Front(bool doPop)
{
    UniqueLock<Mutex> guard(mutex);

    if (WaitNonEmpty(guard))
        return T();

    T res = deque.front();

    if (doPop == true)
    {
        deque.pop_front();
    }

    isEmpty = deque.empty();

    return res;
}

template <typename T>
T ConcurrentDeque<T>::Back(bool doPop)
{
    UniqueLock<Mutex> guard(mutex);

    if (WaitNonEmpty(guard))
        return T();

    T res = deque.back();

    if (doPop == true)
    {
        deque.pop_back();
    }

    isEmpty = deque.empty();

    return res;
}

template <typename T>
size_t ConcurrentDeque<T>::Size() const
{
    UniqueLock<Mutex> guard(mutex);
    return deque.size();
}

template <typename T>
bool ConcurrentDeque<T>::Empty() const
{
    return isEmpty;
}

template <typename T>
void ConcurrentDeque<T>::Clear()
{
    UniqueLock<Mutex> guard(mutex);
    deque.clear();
    isEmpty = true;
}

template <typename T>
void ConcurrentDeque<T>::Cancel()
{
    UniqueLock<Mutex> guard(mutex);
    canceled = true;
    conditionVar.NotifyAll();
}

template <typename T>
bool ConcurrentDeque<T>::IsCanceled() const
{
    return canceled.Get();
}
} // namespace DAVA
