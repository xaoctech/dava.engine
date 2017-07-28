#include "Classes/UserNodeModule/UserNodeModule.h"
#include "Classes/UserNodeModule/Private/UserNodeSystem.h"
#include "Classes/UserNodeModule/Private/UserNodeData.h"

#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <FileSystem/FilePath.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Material/NMaterialNames.h>
#include <Render/Material/NMaterial.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Scene.h>
#include <Scene3D/SceneFileV2.h>
#include <Scene3D/Components/ComponentHelpers.h>

namespace UserNodeModuleDetails
{
DAVA::RenderObject* CreateRenderObject()
{
    using namespace DAVA;

    FilePath scenePath = "~res:/ResourceEditor/3DObjects/Botspawn/tanksbox.sc2";

    ScopedPtr<Scene> scene(new Scene());
    SceneFileV2::eError result = scene->LoadScene(scenePath);
    if (result == SceneFileV2::ERROR_NO_ERROR)
    {
        Vector<Entity*> entities;
        scene->GetChildEntitiesWithComponent(entities, Component::RENDER_COMPONENT);
        if (entities.size() == 1)
        {
            RenderObject* ro = GetRenderObject(entities[0]);
            if (ro != nullptr)
            {
                uint32 count = ro->GetRenderBatchCount();
                for (uint32 i = 0; i < count; ++i)
                {
                    NMaterial* mat = ro->GetRenderBatch(i)->GetMaterial();
                    if (mat != nullptr)
                    {
                        mat->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, 1);
                        mat->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, Color().color, rhi::ShaderProp::TYPE_FLOAT4);
                    }
                }
            }

            return SafeRetain(ro);
        }
    }

    Logger::Error("[%s] Can't open scene %s properly", __FUNCTION__, scenePath.GetStringValue().c_str());
    return nullptr;
}
}

void UserNodeModule::PostInit()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    QtAction* action = new QtAction(GetAccessor(), QIcon(":/QtIcons/user_object.png"), QString("Custom UserNode Drawing Enabled"));
    { // checked-unchecked and text
        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(UserNodeData::drawingEnabledPropertyName);
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<UserNodeData>();
        action->SetStateUpdationFunction(QtAction::Checked, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            return value.Get<bool>(false);
        });
        action->SetStateUpdationFunction(QtAction::Text, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            if (value.Get<bool>(false))
                return DAVA::String("Custom UserNode Drawing Enabled");
            return DAVA::String("Custom UserNode Drawing Disabled");
        });
    }

    { // enabled/disabled state
        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
        });
    }

    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&UserNodeModule::ChangeDrawingState, this));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateStatusbarPoint(true, 0, { InsertionParams::eInsertionMethod::AfterItem, "actionShowStaticOcclusion" }));

    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

void UserNodeModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    if (spawnObject.get() == nullptr)
    { //we cannot create scene at post init, so I call it here
        spawnObject = UserNodeModuleDetails::CreateRenderObject();
    }

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<UserNodeData> userData = std::make_unique<UserNodeData>();
    userData->system.reset(new UserNodeSystem(scene, spawnObject.get()));
    userData->system->DisableSystem();
    scene->AddSystem(userData->system.get(), MAKE_COMPONENT_MASK(DAVA::Component::USER_COMPONENT), DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    context->CreateData(std::move(userData));
}

void UserNodeModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    UserNodeData* userData = context->GetData<UserNodeData>();
    scene->RemoveSystem(userData->system.get());
}

void UserNodeModule::ChangeDrawingState()
{
    DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    UserNodeData* moduleData = context->GetData<UserNodeData>();

    bool enabled = moduleData->IsDrawingEnabled();
    moduleData->SetDrawingEnabled(!enabled);
}

DAVA_VIRTUAL_REFLECTION_IMPL(UserNodeModule)
{
    DAVA::ReflectionRegistrator<UserNodeModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(UserNodeModule);
