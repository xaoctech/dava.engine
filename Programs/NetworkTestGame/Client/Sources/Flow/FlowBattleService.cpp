#include "Battle.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Flow/FlowBattleService.h"
#include "GameClient.h"

#include <FileSystem/KeyedArchive.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <UI/Flow/UIFlowContext.h>

DAVA_VIRTUAL_REFLECTION_IMPL(FlowBattleService)
{
    DAVA::ReflectionRegistrator<FlowBattleService>::Begin()
    .ConstructorByValue()
    .ConstructorByPointer()
    .DestructorByPointer([](FlowBattleService* b) { DAVA::SafeDelete(b); })
    .Field("battleScene", &FlowBattleService::GetBattleScene, nullptr)
    .Field("gameClient", &FlowBattleService::GetGameClient, nullptr)
    .Field("battle", &FlowBattleService::GetBattle, nullptr)
    .Method("Update", &FlowBattleService::Update)
    .End();
}

void FlowBattleService::Activate(DAVA::UIFlowContext* context)
{
    BattleOptions options = BattleOptions::FromKeyedArchive(context->GetData()->GetArchive("clientOptions"));

    battle = std::make_unique<Battle>();
    battle->Initialize(options);
}

void FlowBattleService::Deactivate(DAVA::UIFlowContext* context)
{
    battle.reset();
}

void FlowBattleService::Update(DAVA::float32 frameDelta)
{
    if (battle)
    {
        battle->Update(frameDelta);
    }
}

GameClient* FlowBattleService::GetGameClient() const
{
    return battle ? battle->GetClient() : nullptr;
}

DAVA::Scene* FlowBattleService::GetBattleScene() const
{
    return battle ? battle->GetBattleScene() : nullptr;
}

Battle* FlowBattleService::GetBattle() const
{
    return battle.get();
}
