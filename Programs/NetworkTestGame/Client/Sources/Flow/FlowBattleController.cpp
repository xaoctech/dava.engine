#include "Flow/FlowBattleController.h"
#include "Flow/FlowBattleService.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "GameClient.h"
#include "Battle.h"

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimelineSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkResimulationSystem.h>

#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>
#include <UI/Flow/UIFlowContext.h>
#include <UI/Text/UITextComponent.h>
#include <UI/UI3DView.h>
#include <UI/UIControl.h>
#include <UI/Joypad/UIJoypadComponent.h>

#include <iomanip>

DAVA_VIRTUAL_REFLECTION_IMPL(FlowBattleController)
{
    DAVA::ReflectionRegistrator<FlowBattleController>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](FlowBattleController* c) { DAVA::SafeDelete(c); })
    .End();
}

void FlowBattleController::Activate(DAVA::UIFlowContext* context, DAVA::UIControl* view)
{
    using namespace DAVA;

    battleService = context->GetService<FlowBattleService>(FastName("FlowBattleService"));
    DVASSERT(battleService);

    battleScene = battleService->GetBattleScene();
    DVASSERT(battleScene);

    UIControl* leftJoypad = view->FindByPath("HUD/JoypadMovement");
    DVASSERT(leftJoypad);

    movementJoypad = leftJoypad->GetComponent<UIJoypadComponent>();
    DVASSERT(movementJoypad);
    movementJoypad->SetActivationThreshold(.1f);
    movementJoypad->SetCoordsTransformFunction(UIJoypadComponent::CoordsTransformFn([](Vector2 v) {
        if (std::fabs(v.x) > 0.1f || std::fabs(v.y) > 0.1f)
        {
            // For some reason 'y' should be inverted.
            return Vector2{ v.x, -v.y };
        }
        return Vector2::Zero;
    }));

    UI3DView* ui3dView = view->FindByPath<UI3DView*>("3DView");
    DVASSERT(ui3dView);
    ui3dView->SetScene(battleScene);

    UIControl* pingText = view->FindByPath("HUD/InfoPanel/*/PingText");
    DVASSERT(pingText);
    pingTextComponent = pingText->GetOrCreateComponent<UITextComponent>();
    DVASSERT(pingTextComponent);

    UIControl* lossText = view->FindByPath("HUD/InfoPanel/*/LossText");
    DVASSERT(lossText);
    lossTextComponent = lossText->GetOrCreateComponent<UITextComponent>();
    DVASSERT(pingTextComponent);

    UIControl* frameControl = view->FindByPath("HUD/InfoPanel/*/FrameText");
    DVASSERT(frameControl);
    frameTextComponent = frameControl->GetOrCreateComponent<UITextComponent>();
    DVASSERT(frameTextComponent);

    UIControl* diffControl = view->FindByPath("HUD/InfoPanel/*/FDiffText");
    DVASSERT(diffControl);
    diffTextComponent = diffControl->GetOrCreateComponent<UITextComponent>();
    DVASSERT(diffTextComponent);

    UIControl* pauseControl = view->FindByPath("HUD/InfoPanel/*/PauseText");
    DVASSERT(pauseControl);
    pauseTextComponent = pauseControl->GetOrCreateComponent<UITextComponent>();
    DVASSERT(pauseTextComponent);

    UIControl* incorrectInputControl = view->FindByPath("HUD/InfoPanel/*/IncorrectInputText");
    DVASSERT(incorrectInputControl);
    incorrectInputTextComponent = incorrectInputControl->GetOrCreateComponent<UITextComponent>();
    DVASSERT(incorrectInputTextComponent);

    UIControl* resimulationsControl = view->FindByPath("HUD/InfoPanel/*/NumResimulationsText");
    DVASSERT(resimulationsControl);
    resimulationsCountTextComponent = resimulationsControl->GetOrCreateComponent<UITextComponent>();
    DVASSERT(resimulationsCountTextComponent);

    BattleControls& controls = battleScene->GetSingletonComponent<BattleOptionsSingleComponent>()->controls;
    controls.currentAim = view->FindByPath("HUD/CurrentAim");
    controls.interactControl = view->FindByPath("HUD/InteractControl");
    controls.finalAim = view->FindByPath("HUD/FinalAim");
    controls.movementJoypad = movementJoypad;

    // TODO: make it via ui editor
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    Size2i windowSize = vcs->GetVirtualScreenSize();
    controls.finalAim->SetPosition(Vector2(windowSize.dx / 2.0f, windowSize.dy / 2.0f));
}

void FlowBattleController::Deactivate(DAVA::UIFlowContext* context, DAVA::UIControl* view)
{
    BattleControls& controls = battleScene->GetSingletonComponent<BattleOptionsSingleComponent>()->controls;
    controls.finalAim = nullptr;
    controls.currentAim = nullptr;
    controls.interactControl = nullptr;

    battleService = nullptr;
    battleScene = nullptr;
    movementJoypad = nullptr;
    pingTextComponent = nullptr;
    lossTextComponent = nullptr;
    frameTextComponent = nullptr;
    diffTextComponent = nullptr;
    pauseTextComponent = nullptr;
}

void FlowBattleController::Process(DAVA::float32 frameDelta)
{
    using namespace DAVA;

    battleService->Update(frameDelta);

    if (!battleScene)
        return;

    ActionsSingleComponent* actionsSingleComponent = battleScene->GetSingletonComponent<ActionsSingleComponent>();

    Vector2 pos = movementJoypad->GetTransformedCoords();
    if (pos != Vector2::Zero)
    {
        actionsSingleComponent->AddAnalogAction(FastName("LMOVE"), pos, actionsSingleComponent->GetLocalPlayerId());
    }

    // Update info
    GameClient* client = battleService->GetGameClient();
    DAVA::UDPClient& udpClient = client->GetUDPClient();

    NetworkTimeSingleComponent* netTimeComp = battleScene->GetSingletonComponent<NetworkTimeSingleComponent>();
    int32 frameDiff = netTimeComp->GetClientServerDiff(udpClient.GetAuthToken());
    float32 a = 2.f / (NetworkTimeSingleComponent::FrequencyHz + 1.f);
    diffAvg = a * frameDiff + (1.f - a) * diffAvg;

    if (udpClient.IsConnected())
    {
        int32 halfRtt = udpClient.GetPing() / (NetworkTimeSingleComponent::FrameDurationMs * 2);
        frameTextComponent->SetText(std::to_string(netTimeComp->GetFrameId()));
        diffTextComponent->SetText(std::to_string(static_cast<int32>(std::round(diffAvg) + halfRtt)));
        pingTextComponent->SetText(std::to_string(udpClient.GetPing()));
        lossTextComponent->SetText(std::to_string(udpClient.GetPacketLoss() * 100.f));

        NetworkRemoteInputSystem* remoteInputSystem = battleScene->GetSystem<NetworkRemoteInputSystem>();
        if (remoteInputSystem && remoteInputSystem->GetFullInputComparisonFlag())
        {
            std::stringstream ss;
            ss << std::setprecision(3) << remoteInputSystem->GetIncorrectServerFramesPercentage() * 100.0f << "%";
            incorrectInputTextComponent->SetText(ss.str());
        }
        else
        {
            incorrectInputTextComponent->SetText("disabled");
        }

        NetworkResimulationSystem* resimSystem = battleScene->GetSystem<NetworkResimulationSystem>();
        if (resimSystem)
        {
            resimulationsCountTextComponent->SetText(std::to_string(resimSystem->GetResimulationsCount()));
        }
    }
    else
    {
        frameTextComponent->SetText("undefined");
        diffTextComponent->SetText("undefined");
        pingTextComponent->SetText("not connected");
        lossTextComponent->SetText("not connected");
        incorrectInputTextComponent->SetText("not connected");
    }

    if (battleScene->IsFixedUpdatePaused())
    {
        pauseTextComponent->SetText("yes");
    }
    else
    {
        pauseTextComponent->SetText("no");
    }
}

bool FlowBattleController::ProcessEvent(const DAVA::FastName& event, const DAVA::Vector<DAVA::Any>& params)
{
    using namespace DAVA;

    static const FastName FIRST_SHOOT("FIRE_A");
    static const FastName SECOND_SHOOT("FIRE_B");
    static const FastName CLIENT_PAUSE("CLIENT_PAUSE");
    static const FastName SERVER_PAUSE("SERVER_PAUSE");
    static const FastName STEP_OVER("STEP_OVER");

    ActionsSingleComponent* actionsSingleComponent = battleScene->GetSingletonComponent<ActionsSingleComponent>();

    if (event == FIRST_SHOOT)
    {
        actionsSingleComponent->AddDigitalAction(FastName("FIRST_SHOOT"), actionsSingleComponent->GetLocalPlayerId());
        return true;
    }
    else if (event == SECOND_SHOOT)
    {
        actionsSingleComponent->AddDigitalAction(FastName("SECOND_SHOOT"), actionsSingleComponent->GetLocalPlayerId());
        return true;
    }
    else if (event == CLIENT_PAUSE)
    {
        NetworkTimelineSingleComponent* netTimelineComp = battleScene->GetSingletonComponent<NetworkTimelineSingleComponent>();
        if (netTimelineComp)
        {
            netTimelineComp->SetClientJustPaused(true);
        }
        return true;
    }
    else if (event == SERVER_PAUSE)
    {
        NetworkTimelineSingleComponent* netTimelineComp = battleScene->GetSingletonComponent<NetworkTimelineSingleComponent>();
        if (netTimelineComp)
        {
            netTimelineComp->SetServerJustPaused(true);
        }
        return true;
    }
    else if (event == STEP_OVER)
    {
        NetworkTimelineSingleComponent* netTimelineComp = battleScene->GetSingletonComponent<NetworkTimelineSingleComponent>();
        if (netTimelineComp)
        {
            netTimelineComp->SetStepOver(true);
        }
        return true;
    }

    return false;
}
