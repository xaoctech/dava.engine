#pragma once

#include <Concurrency/Mutex.h>
#include <Entity/SingletonComponent.h>
#include <Game.h>

namespace DAVA
{
class Entity;
}

class StatsLoggingSingleComponent : public DAVA::SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(StatsLoggingSingleComponent, DAVA::SingletonComponent);

public:
    void Clear() override;

    bool IsEmpty();
    void AddMessage(const DAVA::String& msg);
    DAVA::String PopMessage();

private:
    DAVA::Mutex mutex;
    DAVA::Deque<DAVA::String> logDeque;
};
