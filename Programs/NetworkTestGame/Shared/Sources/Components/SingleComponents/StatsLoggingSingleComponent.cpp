#include "StatsLoggingSingleComponent.h"

#include <Concurrency/LockGuard.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(StatsLoggingSingleComponent)
{
    ReflectionRegistrator<StatsLoggingSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

bool StatsLoggingSingleComponent::IsEmpty()
{
    return logDeque.empty();
}

void StatsLoggingSingleComponent::AddMessage(const DAVA::String& msg)
{
    LockGuard<Mutex> lock(mutex);
    logDeque.push_back(msg);
}

DAVA::String StatsLoggingSingleComponent::PopMessage()
{
    LockGuard<Mutex> lock(mutex);
    DAVA::String msg(std::move(logDeque.front()));
    logDeque.pop_front();

    return msg;
}

void StatsLoggingSingleComponent::Clear()
{
    LockGuard<Mutex> lock(mutex);
    logDeque.clear();
}
