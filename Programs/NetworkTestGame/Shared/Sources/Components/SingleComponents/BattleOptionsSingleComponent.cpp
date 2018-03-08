#include "BattleOptionsSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <FileSystem/KeyedArchive.h>

using namespace DAVA;

BattleOptions BattleOptions::FromKeyedArchive(DAVA::KeyedArchive* archive)
{
    BattleOptions options;
    if (archive)
    {
        options.token = archive->GetFastName("token");
        options.hostName = archive->GetString("hostName");
        options.port = archive->GetUInt32("port");
        options.playerKind = PlayerKind(archive->GetUInt32("playerKind"));
        options.isDebug = archive->GetBool("isDebug");
        options.freqHz = archive->GetUInt32("freqHz");
        options.gameStatsLogPath = archive->GetString("gameStatsLogPath");
    }
    return options;
}

DAVA_VIRTUAL_REFLECTION_IMPL(BattleOptionsSingleComponent)
{
    ReflectionRegistrator<BattleOptionsSingleComponent>::Begin()[DAVA::M::Replicable(DAVA::M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("GameModeId", &BattleOptionsSingleComponent::gameModeId)[M::Replicable()]
    .Field("IsEnemyPredicted", &BattleOptionsSingleComponent::isEnemyPredicted)[M::Replicable()]
    .Field("isEnemyRewound", &BattleOptionsSingleComponent::isEnemyRewound)[M::Replicable()]
    .Field("CompareInputs", &BattleOptionsSingleComponent::compareInputs)[M::Replicable()]
    .Field("CollisionResolveMode", &BattleOptionsSingleComponent::collisionResolveMode)[M::Replicable()]
    .End();
}
