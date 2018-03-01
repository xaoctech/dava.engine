#pragma once

#include <Base/BaseTypes.h>
#include <Concurrency/Thread.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Logger/Logger.h>

#include <memory>

using namespace DAVA;

class StatsLoggingSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(StatsLoggingSystem, SceneSystem);

    explicit StatsLoggingSystem(Scene* scene);
    ~StatsLoggingSystem() override;

private:
    Thread* logThread = nullptr;
};
