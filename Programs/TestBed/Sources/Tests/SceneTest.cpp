#include "Tests/SceneTest.h"
#include "Infrastructure/TestBed.h"

#include <Base/ScopedPtr.h>
#include <Debug/DVAssert.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <UI/Scene3D/UIEntityMarkerComponent.h>
#include <UI/Scene3D/UIEntityMarkerSystem.h>
#include <UI/Text/UITextComponent.h>
#include <UI/UI3DView.h>
#include <UI/UIControl.h>
#include <Utils/StringFormat.h>

using namespace DAVA;

SceneTest::SceneTest(TestBed& app)
    : BaseScreen(app, "SceneTest")
{
}

void SceneTest::LoadResources()
{
    BaseScreen::LoadResources();

    // Load UI
    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/SceneTest.yaml", &pkgBuilder);
    UIControl* dialog = pkgBuilder.GetPackage()->GetControl("Main");
    AddControl(dialog);
    BringChildBack(dialog);

    // Load scene
    ScopedPtr<Scene> scene(new Scene());
    scene->LoadScene("~res:/3d/simple_scene.sc2");

    scene->AddSystem(new RotationControllerSystem(scene),
                     MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT),
                     Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);
    scene->AddSystem(new WASDControllerSystem(scene),
                     MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT),
                     Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    Entity* cameraNode = scene->FindByName("Camera");
    if (cameraNode)
    {
        CameraComponent* cc = static_cast<CameraComponent*>(cameraNode->GetComponent(Component::CAMERA_COMPONENT));
        if (cc)
        {
            Camera* camera = cc->GetCamera();
            VirtualCoordinatesSystem* vcs = DAVA::GetEngineContext()->uiControlSystem->vcs;
            float32 aspect = static_cast<float32>(vcs->GetVirtualScreenSize().dx) / vcs->GetVirtualScreenSize().dy;
            camera->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
            scene->SetCurrentCamera(camera);
        }
    }

    // Setup UI
    UI3DView* ui3dView = dialog->FindByPath<UI3DView*>("UI3DView");
    DVASSERT(ui3dView);
    ui3dView->SetScene(scene);

    UIControl* markers = dialog->FindByPath("Markers");
    DVASSERT(markers);
    Vector<Entity*> entities;
    //scene->GetChildEntitiesWithComponent(entities, Component::SCREEN_POSITION_COMPONENT);
    scene->GetChildNodes(entities);
    for (Entity* e : entities)
    {
        RefPtr<UIControl> marker = pkgBuilder.GetPackage()->GetPrototype(FastName("Marker"))->SafeClone();
        DVASSERT(marker.Valid());
        UIControl* titleCtrl = marker->FindByPath("Title");
        DVASSERT(titleCtrl);
        UITextComponent* titleText = titleCtrl->GetComponent<UITextComponent>();
        DVASSERT(titleText);
        titleText->SetText(e->GetName().c_str());

        UIEntityMarkerComponent* emc = marker->GetComponent<UIEntityMarkerComponent>();
        DVASSERT(emc);
        emc->SetTargetEntity(e);
        emc->SetCustomStrategy([](UIControl* ctrl, UIEntityMarkerComponent* emc) {
            UIControl* distanceCtrl = ctrl->FindByPath("Distance");
            DVASSERT(distanceCtrl);
            UITextComponent* distanceText = distanceCtrl->GetComponent<UITextComponent>();
            DVASSERT(distanceText);
            float32 distance = (emc->GetTargetEntity()->GetScene()->GetCurrentCamera()->GetPosition() - emc->GetTargetEntity()->GetWorldTransform().GetTranslationVector()).Length();
            distanceText->SetText(Format("%.3f", distance));
        });

        markers->AddControl(marker.Get());
    }
}

void SceneTest::UnloadResources()
{
    BaseScreen::UnloadResources();
}
