#include "Tests/SceneTest.h"
#include "Infrastructure/TestBed.h"

#include <Base/ScopedPtr.h>
#include <Debug/DVAssert.h>
#include <Entity/ComponentUtils.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <UI/Scene3D/UIEntityMarkerComponent.h>
#include <UI/Scene3D/UIEntityMarkersContainerComponent.h>
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
    DAVA::UIPackageLoader().LoadPackage("~res:/TestBed/UI/SceneTest.yaml", &pkgBuilder);
    UIControl* dialog = pkgBuilder.GetPackage()->GetControl("Main");
    AddControl(dialog);
    BringChildBack(dialog);

    // Load scene
    ScopedPtr<Scene> scene(new Scene());
    scene->LoadScene("~res:/TestBed/3d/simple_scene.sc2");

    scene->AddSystem(new RotationControllerSystem(scene),
                     ComponentUtils::MakeMask<CameraComponent>() | ComponentUtils::MakeMask<RotationControllerComponent>(),
                     Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);
    scene->AddSystem(new WASDControllerSystem(scene),
                     ComponentUtils::MakeMask<CameraComponent>() | ComponentUtils::MakeMask<WASDControllerComponent>(),
                     Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    Entity* cameraNode = scene->FindByName("Camera");
    if (cameraNode)
    {
        CameraComponent* cc = static_cast<CameraComponent*>(cameraNode->GetComponent<CameraComponent>());
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
    UIEntityMarkersContainerComponent* emcc = markers->GetComponent<UIEntityMarkersContainerComponent>();
    DVASSERT(emcc);
    emcc->SetCustomStrategy([](UIControl* ctrl, UIEntityMarkersContainerComponent* container, UIEntityMarkerComponent* marker) {
        UIControl* distanceCtrl = ctrl->FindByPath("Distance");
        DVASSERT(distanceCtrl);
        UITextComponent* distanceText = distanceCtrl->GetComponent<UITextComponent>();
        DVASSERT(distanceText);
        Vector3 camPos = marker->GetTargetEntity()->GetScene()->GetCurrentCamera()->GetPosition();
        Vector3 entPos = marker->GetTargetEntity()->GetWorldTransform().GetTranslationVector();
        float32 distance = (camPos - entPos).Length();
        distanceText->SetText(Format("%.3f", distance));
    });

    Vector<Entity*> entities;
    scene->GetChildNodes(entities);
    UIControl* markerProto = pkgBuilder.GetPackage()->GetPrototype(FastName("Marker"));
    DVASSERT(markerProto);
    for (Entity* e : entities)
    {
        RefPtr<UIControl> marker = markerProto->SafeClone();
        DVASSERT(marker.Valid());
        UIControl* titleCtrl = marker->FindByPath("Title");
        DVASSERT(titleCtrl);
        UITextComponent* titleText = titleCtrl->GetComponent<UITextComponent>();
        DVASSERT(titleText);
        titleText->SetText(e->GetName().c_str());

        UIEntityMarkerComponent* emc = marker->GetComponent<UIEntityMarkerComponent>();
        DVASSERT(emc);
        emc->SetTargetEntity(e);

        markers->AddControl(marker.Get());
    }
}

void SceneTest::UnloadResources()
{
    BaseScreen::UnloadResources();
}
