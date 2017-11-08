#include "Classes/SceneManager/EntityProducerModule.h"

#include <REPlatform/Commands/EntityAddCommand.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Global/StringConstants.h>
#include <REPlatform/Scene/SceneEditor2.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Functional/Function.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/Controller/RotationControllerComponent.h>
#include <Scene3D/Components/Controller/WASDControllerComponent.h>
#include <Scene3D/Entity.h>

void EntityProducerModule::PostInit()
{
    using namespace DAVA;

    QtAction* instantiateCamera = new QtAction(GetAccessor(), "Clone Current Camera");
    FieldDescriptor fieldDescriptor;
    fieldDescriptor.type = DAVA::ReflectedTypeDB::Get<SceneData>();
    fieldDescriptor.fieldName = DAVA::FastName(SceneData::scenePropertyName);
    instantiateCamera->SetStateUpdationFunction(QtAction::Enabled, fieldDescriptor, [](const DAVA::Any& value) -> DAVA::Any
                                                {
                                                    return value.CanCast<SceneData::TSceneType>();
                                                });

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(QList<QString>() << "menuCreateNode"
                                                                     << "menuAdd"));
    GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, instantiateCamera);
    connections.AddConnection(instantiateCamera, &QAction::triggered, DAVA::MakeFunction(this, &EntityProducerModule::InstantiateCurrentCamera));
}

void EntityProducerModule::InstantiateCurrentCamera()
{
    using namespace DAVA;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    DAVA::RefPtr<SceneEditor2> sceneEditor = sceneData->GetScene();

    DAVA::Camera* currentCamera = sceneEditor->GetCurrentCamera();
    DVASSERT(currentCamera != nullptr);

    DAVA::ScopedPtr<DAVA::Camera> camera(static_cast<DAVA::Camera*>(currentCamera->Clone()));

    DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
    sceneNode->AddComponent(new DAVA::CameraComponent(camera));
    sceneNode->AddComponent(new DAVA::WASDControllerComponent());
    sceneNode->AddComponent(new DAVA::RotationControllerComponent());
    sceneNode->SetName(DAVA::FastName(DAVA::ResourceEditor::CAMERA_NODE_NAME));

    sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new DAVA::EntityAddCommand(sceneNode, sceneEditor.Get())));
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityProducerModule)
{
    DAVA::ReflectionRegistrator<EntityProducerModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(EntityProducerModule);