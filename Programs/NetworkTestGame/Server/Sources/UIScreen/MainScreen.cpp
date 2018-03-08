#include "MainScreen.h"

#include "Time/SystemTimer.h"
#include "UI/Update/UIUpdateComponent.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"

#include "NetworkCore/Scene3D/Systems/NetworkTimeSystem.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

using namespace DAVA;

MainScreen::MainScreen(Scene* scene_)
    : scene(scene_)
{
    actionsSingleComponent = scene_->GetSingleComponent<ActionsSingleComponent>();
}

void MainScreen::LoadResources()
{
    if (scene)
    {
        DVASSERT(ui3DView.get() == nullptr);

        ui3DView.reset(new UI3DView(GetRect()));
        AddControl(ui3DView);
        ui3DView->SetScene(scene);
    }

    AddJoypadControl();
}

void MainScreen::UnloadResources()
{
    RemoveControl(ui3DView);
    ui3DView.reset(nullptr);
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
