#include "MainScreen.h"

#include "Time/SystemTimer.h"
#include "UI/Update/UIUpdateComponent.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"

#include "NetworkCore/Scene3D/Systems/NetworkTimeSystem.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

using namespace DAVA;

MainScreen::MainScreen(const ScopedPtr<Scene>& scene_)
    : scene(scene_)
{
    actionsSingleComponent = scene_->GetSingletonComponent<ActionsSingleComponent>();
}

void MainScreen::LoadResources()
{
    if (scene)
    {
        ScopedPtr<UI3DView> sceneView(new UI3DView(GetRect()));
        AddControl(sceneView);
        sceneView->SetScene(scene);
    }

    AddJoypadControl();
}

void MainScreen::UnloadResources()
{
    RemoveControl(moveJoyPAD);
    SafeRelease(moveJoyPAD);
}

void MainScreen::Update(DAVA::float32 timeElapsed)
{
    if (moveJoyPAD)
    {
        Vector2 joypadPos = moveJoyPAD->GetAnalogPosition();
        actionsSingleComponent->AddAnalogAction(FastName("LMOVE"), joypadPos);
    }
}

void MainScreen::AddJoypadControl()
{
    moveJoyPAD = new UIJoypad(DAVA::Rect(10, GetRect().dy - 210.f, 200.f, 200.f));
    DAVA::ScopedPtr<DAVA::Sprite> stickSprite(DAVA::Sprite::CreateFromSourceFile("~res:/UI/Joypad.png", true));
    moveJoyPAD->SetStickSprite(stickSprite, 0);
    AddControl(moveJoyPAD);
}
