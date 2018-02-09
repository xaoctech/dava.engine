#pragma once

#include <Base/ScopedPtr.h>
#include <Reflection/Reflection.h>
#include <UI/Flow/UIFlowService.h>

#include <Functional/Function.h>
#include "Game.h"

namespace DAVA
{
class Scene;
class KeyedArchive;
}

class GameClient;
class BattleOptionsSingleComponent;
struct BattleOptions;
class Battle
{
    DAVA_REFLECTION(Battle);

public:
    using OnInitialize = DAVA::Function<void()>;

    void Initialize(const BattleOptions& options);
    void Release();
    void Update(DAVA::float32 frameDelta);

    DAVA::Scene* GetBattleScene() const;
    GameClient* GetClient() const;

private:
    void CreateBattleScene();
    void FreeBattleScene();

    void SetupTestGame();

    std::unique_ptr<GameClient> gameClient;
    DAVA::ScopedPtr<DAVA::Scene> battleScene = nullptr;
    DAVA::UnorderedSet<DAVA::FastName> tags;
    BattleOptionsSingleComponent* optionsSingleComp;
};

inline DAVA::Scene* Battle::GetBattleScene() const
{
    return battleScene.get();
}

inline GameClient* Battle::GetClient() const
{
    return gameClient.get();
}
