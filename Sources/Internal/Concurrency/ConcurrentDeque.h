#pragma once

#include "Base/Deque.h"
#include "Functional/Function.h"
#include "Concurrency/Atomic.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/UniqueLock.h"
#include "Concurrency/ConditionVariable.h"

namespace DAVA
{
template <typename T>
class ConcurrentDeque
{
public:
    ConcurrentDeque();

    void ProcessDeque(const Function<void(Deque<T>&)>& fn);

    void PushBack(const T& t);
    void PushFront(const T& t);

    T Front(bool doPop);
    T Back(bool doPop);

    size_t Size() const;
    bool Empty() const;
    void Clear();

    void Cancel();
    bool IsCanceled() const;

private:
    bool WaitNonEmpty(UniqueLock<Mutex>& lock);

    Deque<T> deque;
    bool isEmpty;

    Mutex mutex;
    ConditionVariable conditionVar;
    Atomic<bool> canceled;
};
} // namespace DAVA

#define __DAVA_CONCURRENCY_DEQUE__
#include "Concurrency/Private/ConcurrentDeque_impl.h"
