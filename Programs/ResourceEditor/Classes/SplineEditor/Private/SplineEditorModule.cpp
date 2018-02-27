#include "Classes/SplineEditor/SplineEditorModule.h"

#include "Classes/SplineEditor/Private/SplineEditorPPExtensions.h"
#include "Classes/SplineEditor/Private/SplineEditorWidget.h"
#include "Classes/SplineEditor/Private/SplinePointTransformProxy.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/SelectionData.h>
#include <REPlatform/Global/PropertyPanelInterface.h>
#include <REPlatform/Scene/Components/SplineEditorDrawComponent.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/SelectionSystem.h>
#include <REPlatform/Scene/Systems/SplineEditorSystem.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Utils/ReflectionHelpers.h>

#include <Scene3D/Components/SplineComponent.h>

void SplineEditorModule::OnContextCreated(DAVA::DataContext* context)
{
    using namespace DAVA;
    SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
    DVASSERT(scene != nullptr);
    SplineEditorSystem* system = new SplineEditorSystem(scene);
    SelectionSystem* processInputBeforeSystem = scene->GetSystem<SelectionSystem>();
    scene->AddSystem(system, nullptr, processInputBeforeSystem);
}

void SplineEditorModule::OnContextDeleted(DAVA::DataContext* context)
{
    DAVA::SceneEditor2* scene = context->GetData<DAVA::SceneData>()->GetScene().Get();
    DVASSERT(scene != nullptr);
    DAVA::SplineEditorSystem* system = scene->GetSystem<DAVA::SplineEditorSystem>();
    scene->RemoveSystem(system);
    delete system;
}

void SplineEditorModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<DAVA::PropertyPanelInterface>())
    {
        DAVA::PropertyPanelInterface* pp = QueryInterface<DAVA::PropertyPanelInterface>();
        std::shared_ptr<DAVA::ExtensionChain> splineComponentCreator(new SplineComponentChildCreator);
        std::shared_ptr<DAVA::ExtensionChain> editorCreator(new SplineEditorCreator);
        pp->RegisterExtension(splineComponentCreator);
        pp->RegisterExtension(editorCreator);
    }
}

void SplineEditorModule::PostInit()
{
    using namespace DAVA;

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SplineEditorDrawComponent);
    GetEngineContext()->componentManager->RegisterComponent<SplineEditorDrawComponent>();

    Selectable::AddTransformProxyForClass<SplineComponent::SplinePoint, SplinePointTransformProxy>(GetAccessor());
    EmplaceTypeMeta<SplineComponent::SplinePoint>(RemoveSplinePointCommandProducer());

    fieldBinder = std::make_unique<FieldBinder>(GetAccessor());

    {
        FieldDescriptor descr;
        descr.type = ReflectedTypeDB::Get<SelectionData>();
        descr.fieldName = FastName(SelectionData::selectionPropertyName);
        fieldBinder->BindField(descr, DAVA::MakeFunction(this, &SplineEditorModule::OnSelectionChanged));
    }
}

void SplineEditorModule::OnSelectionChanged(const DAVA::Any& selection)
{
    using namespace DAVA;

    DataContext* context = GetAccessor()->GetActiveContext();
    if (context == nullptr || context->GetData<SceneData>() == nullptr)
    {
        return;
    }
    SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
    SplineEditorSystem* splineEditorSystem = scene->GetSystem<SplineEditorSystem>();
    SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();

    struct Selected
    {
        bool wasSelected = false;
        bool nowSelected = false;
    };
    UnorderedMap<SplineEditorDrawComponent*, Selected> selected;

    for (SplineEditorDrawComponent* c : splineEditorSystem->GetSplineDrawComponents())
    {
        if (c->IsSelected())
        {
            selected[c].wasSelected = true;
        }
    }

    SelectableGroup group = selection.Cast<SelectableGroup>(SelectableGroup());
    for (const Selectable& selectable : group.GetContent())
    {
        Entity* e = selectable.AsEntity();
        if (e != nullptr)
        {
            SplineEditorDrawComponent* spline = e->GetComponent<SplineEditorDrawComponent>();
            if (spline != nullptr)
            {
                selected[spline].nowSelected = true;
            }
        }
        else if (selectable.GetObjectType() == ReflectedTypeDB::Get<SplineComponent::SplinePoint>())
        {
            SplineComponent::SplinePoint* selectedPoint = selectable.Cast<SplineComponent::SplinePoint>();
            SplineEditorDrawComponent* spline = splineEditorSystem->GetSplineDrawByPoint(selectedPoint);
            selected[spline].nowSelected = true;
        }
    }

    for (const std::pair<SplineEditorDrawComponent*, Selected>& pair : selected)
    {
        pair.first->SetSelected(pair.second.nowSelected);
        if (pair.second.wasSelected != pair.second.nowSelected)
        {
            collisionSystem->UpdateCollisionObject(Selectable(pair.first->GetEntity()), true);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(SplineEditorModule)
{
    DAVA::ReflectionRegistrator<SplineEditorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(SplineEditorModule);
