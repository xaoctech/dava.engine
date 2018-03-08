#pragma once

#include <Concurrency/Mutex.h>
#include <Entity/SingleComponent.h>
#include <Game.h>

namespace DAVA
{
class Entity;
}

class StatsLoggingSingleComponent : public DAVA::SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(StatsLoggingSingleComponent, DAVA::SingleComponent);

public:
    bool IsEmpty();
    void AddMessage(const DAVA::String& msg);
    DAVA::String PopMessage();

private:
    DAVA::Mutex mutex;
    DAVA::Deque<DAVA::String> logDeque;
};
