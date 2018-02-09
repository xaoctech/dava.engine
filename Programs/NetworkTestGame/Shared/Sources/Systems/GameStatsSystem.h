#pragma once

#include <Base/BaseTypes.h>
#include <Concurrency/Mutex.h>
#include <Concurrency/Thread.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Logger/Logger.h>

#include <memory>

using namespace DAVA;

class GameStatsSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameStatsSystem, SceneSystem);

    GameStatsSystem(Scene* scene);
    ~GameStatsSystem() override;

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

private:
    void CreateLogThread();

    struct AgentInfo
    {
        Entity* entity;
        uint32 ticksUntilProcess;
        int32 lastHealthLogged;
        Vector3 lastPositionLogged;
        FastName token;
    };

    void ProcessAgent(AgentInfo& agent);

    Vector<AgentInfo> agents;

    Mutex mutex;
    Thread* logThread = nullptr;
    Deque<String> logDeque;
};
