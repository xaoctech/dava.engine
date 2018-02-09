#pragma once

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Reflection/Reflection.h>
#include <UI/Flow/UIFlowController.h>

namespace DAVA
{
class Scene;
class UIControl;
class UIFlowContext;
class UIJoypad;
class UITextComponent;
class UIJoypadComponent;
}

class FlowBattleService;

class FlowBattleController : public DAVA::UIFlowController
{
    DAVA_VIRTUAL_REFLECTION(FlowBattleController, DAVA::UIFlowController);

public:
    void Activate(DAVA::UIFlowContext* context, DAVA::UIControl* view) override;
    void Deactivate(DAVA::UIFlowContext* context, DAVA::UIControl* view) override;
    void Process(DAVA::float32 frameDelta) override;
    bool ProcessEvent(const DAVA::FastName& event, const DAVA::Vector<DAVA::Any>& params) override;

private:
    FlowBattleService* battleService = nullptr;
    DAVA::Scene* battleScene = nullptr;
    DAVA::UIJoypadComponent* movementJoypad = nullptr;
    DAVA::UITextComponent* pingTextComponent = nullptr;
    DAVA::UITextComponent* lossTextComponent = nullptr;
    DAVA::UITextComponent* frameTextComponent = nullptr;
    DAVA::UITextComponent* diffTextComponent = nullptr;
    DAVA::UITextComponent* pauseTextComponent = nullptr;
    DAVA::UITextComponent* incorrectInputTextComponent = nullptr;
    DAVA::UITextComponent* resimulationsCountTextComponent = nullptr;
    DAVA::float32 diffAvg = 0.f;
};
