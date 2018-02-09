#include "Systems/GameStatsSystem.h"
#include "Components/HealthComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>
#include <FileSystem/FileSystem.h>

namespace GameStatsSystemDetail
{
const uint32 PROCESS_PERIOD = 120;
const float32 DISTANCE_LOG_TOLERANCE = 20.f;

const FastName& GetToken(Entity* entity)
{
    NetworkReplicationComponent* replComp = entity->GetComponent<NetworkReplicationComponent>();
    NetworkGameModeSingleComponent* netGameModeComp = entity->GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID netPlayerID = replComp->GetNetworkPlayerID();
    return netGameModeComp->GetToken(replComp->GetNetworkPlayerID());
}
}

DAVA_VIRTUAL_REFLECTION_IMPL(GameStatsSystem)
{
    ReflectionRegistrator<GameStatsSystem>::Begin()[M::Tags("log_game_stats")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &GameStatsSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_END, SP::Type::FIXED, 3.0f)]
    .End();
}

GameStatsSystem::GameStatsSystem(Scene* scene)
    :
    SceneSystem(scene,
                ComponentUtils::MakeMask<HealthComponent>() |
                ComponentUtils::MakeMask<TransformComponent>() |
                ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
}

GameStatsSystem::~GameStatsSystem()
{
    if (logThread)
    {
        logThread->Cancel();
        logThread->Join();
        //logThread->Release();
        SafeRelease(logThread);
    }
}

void GameStatsSystem::CreateLogThread()
{
    logThread = Thread::Create([this] {
        Thread* thread = Thread::Current();

        BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingletonComponent<BattleOptionsSingleComponent>();
        DVASSERT(!optionsComp->gameStatsLogPath.empty());

        Logger logger;
        logger.SetLogPathname(optionsComp->gameStatsLogPath);

        do
        {
            while (!logDeque.empty())
            {
                mutex.Lock();
                std::string msg(std::move(logDeque.front()));
                logDeque.pop_front();
                mutex.Unlock();

                logger.Log(Logger::LEVEL_INFO, "GameStatsSystem: %s", msg.c_str());

                Thread::Yield();
            }
            Thread::Sleep(10);

        } while (!thread->IsCancelling());
    });

    logThread->SetName("GameStatLogThread");
    logThread->Start();
}

void GameStatsSystem::ProcessFixed(float32 timeElapsed)
{
    if (!logThread)
    {
        CreateLogThread();
    }

    mutex.Lock();
    for (AgentInfo& agent : agents)
    {
        if (agent.ticksUntilProcess == 0)
        {
            ProcessAgent(agent);
            agent.ticksUntilProcess = GameStatsSystemDetail::PROCESS_PERIOD;
        }

        agent.ticksUntilProcess--;
    }
    mutex.Unlock();
}

void GameStatsSystem::AddEntity(Entity* entity)
{
    agents.emplace_back();
    AgentInfo& agent = agents.back();
    agent.entity = entity;
    agent.ticksUntilProcess = agents.size() % GameStatsSystemDetail::PROCESS_PERIOD;
    agent.lastHealthLogged = -1;
    agent.lastPositionLogged.Set(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
    agent.token = GameStatsSystemDetail::GetToken(entity);
}

void GameStatsSystem::RemoveEntity(Entity* entity)
{
    auto found = std::find_if(agents.begin(), agents.end(), [entity](auto& elt) { return elt.entity == entity; });
    if (found != agents.end())
    {
        agents.erase(found);
    }
}

void GameStatsSystem::ProcessAgent(AgentInfo& agent)
{
    if (agent.lastHealthLogged == 0)
    {
        return;
    }

    char cbuf[256];

    Entity* entity = agent.entity;
    const char* tokenStr = agent.token.c_str();
    NetworkTimeSingleComponent* timeComp = entity->GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();

    uint8 curHealth = entity->GetComponent<HealthComponent>()->GetHealth();
    if (agent.lastHealthLogged != curHealth)
    {
        sprintf(cbuf, "Timestamp %d Token %s Health %d", timeComp->GetUptimeMs(), tokenStr, curHealth);
        // Mutex already locked
        logDeque.emplace_back(cbuf);

        agent.lastHealthLogged = curHealth;
    }

    const Vector3& curPos = entity->GetComponent<TransformComponent>()->GetPosition();
    float dist = Distance(agent.lastPositionLogged, curPos);
    if (dist > GameStatsSystemDetail::DISTANCE_LOG_TOLERANCE || curHealth == 0)
    {
        sprintf(cbuf, "Timestamp %d Token %s Position %f %f %f",
                timeComp->GetUptimeMs(), tokenStr, curPos.x, curPos.y, curPos.z);
        // Mutex already locked
        logDeque.emplace_back(cbuf);

        agent.lastPositionLogged = curPos;
    }
}
