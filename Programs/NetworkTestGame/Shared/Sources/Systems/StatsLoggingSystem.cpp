#include "Systems/StatsLoggingSystem.h"

#include "Components/HealthComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Components/SingleComponents/StatsLoggingSingleComponent.h"

#include <FileSystem/FileSystem.h>
#include <Reflection/ReflectionRegistrator.h>

DAVA_VIRTUAL_REFLECTION_IMPL(StatsLoggingSystem)
{
    ReflectionRegistrator<StatsLoggingSystem>::Begin()[M::Tags("log_game_stats")]
    .ConstructorByPointer<Scene*>()
    .End();
}

StatsLoggingSystem::StatsLoggingSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingletonComponent<BattleOptionsSingleComponent>();
    DVASSERT(!optionsComp->options.gameStatsLogPath.empty());

    StatsLoggingSingleComponent* statsComp = GetScene()->GetSingletonComponent<StatsLoggingSingleComponent>();
    DVASSERT(statsComp);

    logThread = Thread::Create([this, optionsComp, statsComp] {
        Thread* thread = Thread::Current();

        Logger logger;
        logger.SetLogPathname(optionsComp->options.gameStatsLogPath);

        do
        {
            while (!statsComp->IsEmpty())
            {
                String msg = statsComp->PopMessage();
                logger.Log(Logger::LEVEL_INFO, "[StatsLoggingSystem] %s", msg.c_str());

                Thread::Yield();
            }
            Thread::Sleep(10);

        } while (!thread->IsCancelling());
    });

    logThread->SetName("GameStatsLogThread");
    logThread->Start();
}

StatsLoggingSystem::~StatsLoggingSystem()
{
    if (logThread)
    {
        logThread->Cancel();
        logThread->Join();
        //logThread->Release();
        SafeRelease(logThread);
    }
}
