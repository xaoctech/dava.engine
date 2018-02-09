#pragma once

#include <UI/Flow/UIFlowService.h>

class Battle;
class GameClient;

namespace DAVA
{
class Scene;
}

class FlowBattleService : public DAVA::UIFlowService
{
    DAVA_VIRTUAL_REFLECTION(FlowBattleService, UIFlowService);

public:
    void Activate(DAVA::UIFlowContext* context) override;
    void Deactivate(DAVA::UIFlowContext* context) override;
    void Update(DAVA::float32 frameDelta);

    DAVA::Scene* GetBattleScene() const;
    GameClient* GetGameClient() const;
    Battle* GetBattle() const;

private:
    std::unique_ptr<Battle> battle;
};
