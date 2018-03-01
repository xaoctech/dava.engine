#pragma once

#include "Components/SingleComponents/StatsLoggingSingleComponent.h"

#include <Base/BaseTypes.h>
#include <Concurrency/Thread.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

#include <memory>

using namespace DAVA;

class PlayerStatsMonitoringSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PlayerStatsMonitoringSystem, SceneSystem);

    explicit PlayerStatsMonitoringSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

private:
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
    StatsLoggingSingleComponent* statsComp = nullptr;
};
