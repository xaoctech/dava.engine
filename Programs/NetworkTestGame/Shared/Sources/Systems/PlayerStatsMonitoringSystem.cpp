#include "Systems/PlayerStatsMonitoringSystem.h"

#include "Components/HealthComponent.h"

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>

namespace PlayerStatsSystemDetail
{
const uint32 PROCESS_PERIOD = 120;
const float32 DISTANCE_LOG_TOLERANCE = 20.f;

const FastName& GetToken(Entity* entity)
{
    NetworkReplicationComponent* replComp = entity->GetComponent<NetworkReplicationComponent>();
    NetworkGameModeSingleComponent* netGameModeComp = entity->GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    return netGameModeComp->GetToken(replComp->GetNetworkPlayerID());
}
}

DAVA_VIRTUAL_REFLECTION_IMPL(PlayerStatsMonitoringSystem)
{
    ReflectionRegistrator<PlayerStatsMonitoringSystem>::Begin()[M::Tags("monitor_game_stats")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &PlayerStatsMonitoringSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::NORMAL, 9.0f)]
    .End();
}

PlayerStatsMonitoringSystem::PlayerStatsMonitoringSystem(Scene* scene)
    :
    SceneSystem(scene,
                ComponentUtils::MakeMask<HealthComponent>() |
                ComponentUtils::MakeMask<TransformComponent>() |
                ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    statsComp = scene->GetSingleComponent<StatsLoggingSingleComponent>();
    DVASSERT(statsComp);
}

void PlayerStatsMonitoringSystem::Process(float32 timeElapsed)
{
    for (AgentInfo& agent : agents)
    {
        if (agent.ticksUntilProcess == 0)
        {
            ProcessAgent(agent);
            agent.ticksUntilProcess = PlayerStatsSystemDetail::PROCESS_PERIOD;
        }

        agent.ticksUntilProcess--;
    }
}

void PlayerStatsMonitoringSystem::AddEntity(Entity* entity)
{
    agents.emplace_back();
    AgentInfo& agent = agents.back();
    agent.entity = entity;
    agent.ticksUntilProcess = agents.size() % PlayerStatsSystemDetail::PROCESS_PERIOD;
    agent.lastHealthLogged = -1;
    agent.lastPositionLogged.Set(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
    agent.token = PlayerStatsSystemDetail::GetToken(entity);
}

void PlayerStatsMonitoringSystem::RemoveEntity(Entity* entity)
{
    auto found = std::find_if(agents.begin(), agents.end(), [entity](auto& elt) { return elt.entity == entity; });
    if (found != agents.end())
    {
        agents.erase(found);
    }
}

void PlayerStatsMonitoringSystem::ProcessAgent(AgentInfo& agent)
{
    if (agent.lastHealthLogged == 0)
    {
        return;
    }

    Entity* entity = agent.entity;
    const char* tokenStr = agent.token.c_str();
    NetworkTimeSingleComponent* timeComp = entity->GetScene()->GetSingleComponent<NetworkTimeSingleComponent>();

    uint8 curHealth = entity->GetComponent<HealthComponent>()->GetHealth();
    if (agent.lastHealthLogged != curHealth)
    {
        String msg = Format("Timestamp %d Token %s Health %d", timeComp->GetUptimeMs(), tokenStr, curHealth);
        statsComp->AddMessage(msg);

        agent.lastHealthLogged = curHealth;
    }

    const Vector3& curPos = entity->GetComponent<TransformComponent>()->GetPosition();
    float dist = Distance(agent.lastPositionLogged, curPos);
    if (dist > PlayerStatsSystemDetail::DISTANCE_LOG_TOLERANCE || curHealth == 0)
    {
        String msg = Format("Timestamp %d Token %s Position %f %f %f", timeComp->GetUptimeMs(), tokenStr, curPos.x, curPos.y, curPos.z);
        statsComp->AddMessage(msg);

        agent.lastPositionLogged = curPos;
    }
}
