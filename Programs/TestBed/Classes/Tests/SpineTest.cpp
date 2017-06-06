#include "Tests/SpineTest.h"

#include "Infrastructure/TestBed.h"

#include <Base/RefPtr.h>
#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/Spine/UISpineComponent.h>

using namespace DAVA;

SpineTest::SpineTest(TestBed& app)
    : BaseScreen(app, "SpineTest")
{
}

void SpineTest::LoadResources()
{
    BaseScreen::LoadResources();

    RefPtr<UIControl> ctrl(new UIControl(Rect(20, 20, 300, 300)));
    ctrl->SetDebugDraw(true);

    UIControlBackground* bg = ctrl->GetOrCreateComponent<UIControlBackground>();
    if (bg)
    {
        bg->SetDrawType(UIControlBackground::DRAW_BATCH);
    }

    UISpineComponent* sc = ctrl->GetOrCreateComponent<UISpineComponent>();
    if (sc)
    {
        sc->SetSkeletonPath("~res:/UI/Spine/spineboy.json");
        sc->SetAtlasPath("~res:/Gfx/Spine/spineboy.atlas");
        sc->SetAnimationName("walk");
        sc->SetAnimationState(UISpineComponent::PLAYED);
    }

    AddControl(ctrl.Get());
}

void SpineTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}
