#include "Classes/LandscapeEditor/LandscapeEditorModule.h"
#include "Classes/LandscapeEditor/Private/LandscapePropertyPanelExt.h"

#include "Classes/LandscapeEditor/Private/HeightAddSubTool.h"
#include "Classes/LandscapeEditor/Private/HeightAverageTool.h"
#include "Classes/LandscapeEditor/Private/HeightCloneStampTool.h"
#include "Classes/LandscapeEditor/Private/HeightFlattenTool.h"
#include "Classes/LandscapeEditor/Private/HeightNoiseTool.h"
#include "Classes/LandscapeEditor/Private/HeightPushPullTool.h"
#include "Classes/LandscapeEditor/Private/HeightRaiseLowerTool.h"
#include "Classes/LandscapeEditor/Private/HeightSetTool.h"
#include "Classes/LandscapeEditor/Private/HeightSharpenTool.h"
#include "Classes/LandscapeEditor/Private/HeightSmoothTool.h"
#include "Classes/LandscapeEditor/Private/ObjectPlacementTool.h"
#include "Classes/LandscapeEditor/Private/MassObjectCreationComponents.h"
#include "Classes/LandscapeEditor/Private/MassObjectCreationSystem.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/SelectionData.h>
#include <REPlatform/Global/PropertyPanelInterface.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Base/Type.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentManager.h>
#include <Entity/ComponentUtils.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>

REGISTER_CLASS(MassObjectCreationLayer);
REGISTER_CLASS(MassCreatedObjectComponent);

LandscapeEditorModule::LandscapeEditorModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightRaiseLowerTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightPushPullTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightAddSubTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightSetTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightFlattenTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightAverageTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightSmoothTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightSharpenTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightNoiseTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HeightCloneStampTool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ObjectPlacementTool);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MassObjectCreationLayer);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MassCreatedObjectComponent);
}

void LandscapeEditorModule::OnContextCreated(DAVA::DataContext* context)
{
    DAVA::SceneData* data = context->GetData<DAVA::SceneData>();
    DAVA::Scene* scene = data->GetScene().Get();
    DAVA::LandscapeEditorSystemV2* system = new DAVA::LandscapeEditorSystemV2(scene);
    system->SetPropertiesCreatorFn([this](const DAVA::String& propsNodeName) {
        return GetAccessor()->CreatePropertiesNode(propsNodeName);
    });

    scene->AddSystem(system);

    MassObjectCreationSystem* massObjSystem = new MassObjectCreationSystem(scene);
    scene->AddSystem(massObjSystem);
}

void LandscapeEditorModule::OnContextDeleted(DAVA::DataContext* context)
{
    DAVA::SceneData* data = context->GetData<DAVA::SceneData>();
    DAVA::Scene* scene = data->GetScene().Get();
    DAVA::LandscapeEditorSystemV2* system = scene->GetSystem<DAVA::LandscapeEditorSystemV2>();
    scene->RemoveSystem(system);
    DAVA::SafeDelete(system);

    MassObjectCreationSystem* massObjSystem = scene->GetSystem<MassObjectCreationSystem>();
    scene->RemoveSystem(massObjSystem);
    DAVA::SafeDelete(massObjSystem);
}

void LandscapeEditorModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<DAVA::PropertyPanelInterface>())
    {
        DAVA::PropertyPanelInterface* pp = QueryInterface<DAVA::PropertyPanelInterface>();
        std::shared_ptr<DAVA::ExtensionChain> childCreator(new LandscapeEditorChildCreator());
        std::shared_ptr<DAVA::ExtensionChain> editorCreator(new LandscapeEditorCreator());
        pp->RegisterExtension(childCreator);
        pp->RegisterExtension(editorCreator);
    }
}

void LandscapeEditorModule::PostInit()
{
    DAVA::ComponentManager* componentManager = GetAccessor()->GetEngineContext()->componentManager;
    componentManager->RegisterComponent<MassObjectCreationLayer>();
    componentManager->RegisterComponent<MassCreatedObjectComponent>();

    fieldBinder.reset(new DAVA::FieldBinder(GetAccessor()));

    DAVA::FieldDescriptor descr;
    descr.type = DAVA::ReflectedTypeDB::Get<DAVA::SelectionData>();
    descr.fieldName = DAVA::FastName(DAVA::SelectionData::selectionPropertyName);
    fieldBinder->BindField(descr, [this](const DAVA::Any& v) {
        DAVA::DataContext* ctx = GetAccessor()->GetActiveContext();
        if (ctx == nullptr)
        {
            return;
        }

        DAVA::SceneData* data = ctx->GetData<DAVA::SceneData>();
        DAVA::Scene* scene = data->GetScene().Get();
        DAVA::LandscapeEditorSystemV2* system = scene->GetSystem<DAVA::LandscapeEditorSystemV2>();
        DAVA::Landscape* landscapeForEdit = nullptr;
        if (v.CanGet<DAVA::SelectableGroup>())
        {
            const DAVA::SelectableGroup& selection = v.Get<DAVA::SelectableGroup>();
            if (selection.GetSize() == 1)
            {
                const DAVA::Selectable& obj = selection.GetFirst();
                if (obj.CanBeCastedTo<DAVA::Entity>())
                {
                    DAVA::Entity* entity = obj.Cast<DAVA::Entity>();
                    DAVA::RenderObject* ro = DAVA::GetRenderObject(entity);
                    if (ro != nullptr && ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE)
                    {
                        landscapeForEdit = static_cast<DAVA::Landscape*>(ro);
                    }
                }
            }
        }

        system->PrepareForEdit(landscapeForEdit);
    });
}

DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeEditorModule)
{
    DAVA::ReflectionRegistrator<LandscapeEditorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(LandscapeEditorModule);
