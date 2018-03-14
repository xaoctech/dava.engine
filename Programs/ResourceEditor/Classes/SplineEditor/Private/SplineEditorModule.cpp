#include "Classes/SplineEditor/SplineEditorModule.h"

#include "Classes/SplineEditor/Private/SplineEditorPPExtensions.h"
#include "Classes/SplineEditor/Private/SplineEditorWidget.h"
#include "Classes/SplineEditor/Private/SplinePointTransformProxy.h"

#include <REPlatform/Commands/SplineEditorCommands.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/SelectionData.h>
#include <REPlatform/Global/PropertyPanelInterface.h>
#include <REPlatform/Scene/Components/SplineEditorDrawComponent.h>
#include <REPlatform/Scene/Systems/CameraSystem.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/SelectionSystem.h>
#include <REPlatform/Scene/Systems/SplineEditorSystem.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Utils/ReflectionHelpers.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Scene3D/Components/SplineComponent.h>

struct SplineEditorData : public DAVA::TArcDataNode
{
    std::unique_ptr<DAVA::SplineEditorSystem> splineEditorSystem = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SplineEditorData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<SplineEditorData>::Begin()
        .End();
    }
};

void SplineEditorModule::OnContextCreated(DAVA::DataContext* context)
{
    using namespace DAVA;

    SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<SplineEditorData> data = std::make_unique<SplineEditorData>();
    data->splineEditorSystem = std::make_unique<DAVA::SplineEditorSystem>(scene);
    SplineEditorSystem* system = data->splineEditorSystem.get();
    context->CreateData(std::move(data));

    SelectionSystem* selectionSystem = scene->GetSystem<SelectionSystem>();
    selectionSystem->AddDelegate(system);

    SceneCameraSystem* processInputBeforeSystem = scene->GetSystem<SceneCameraSystem>();
    scene->AddSystem(system, nullptr, processInputBeforeSystem);
}

void SplineEditorModule::OnContextDeleted(DAVA::DataContext* context)
{
    using namespace DAVA;

    SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
    DVASSERT(scene != nullptr);
    SplineEditorSystem* system = scene->GetSystem<SplineEditorSystem>();

    SelectionSystem* selectionSystem = scene->GetSystem<SelectionSystem>();
    if (selectionSystem != nullptr)
    {
        selectionSystem->RemoveDelegate(system);
    }

    scene->RemoveSystem(system);
}

void SplineEditorModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<DAVA::PropertyPanelInterface>())
    {
        DAVA::PropertyPanelInterface* pp = QueryInterface<DAVA::PropertyPanelInterface>();

        std::shared_ptr<DAVA::ExtensionChain> splineComponentChildCreator(new SplineComponentChildCreator);
        pp->RegisterExtension(splineComponentChildCreator);

        std::shared_ptr<DAVA::ExtensionChain> splineComponentEditorCreator(new SplineEditorCreator);
        pp->RegisterExtension(splineComponentEditorCreator);

        std::shared_ptr<DAVA::ExtensionChain> splinePointChildCreator(new SplinePointChildCreator);
        pp->RegisterExtension(splinePointChildCreator);

        std::shared_ptr<DAVA::ExtensionChain> splinePointEditorCreator(new SplinePointEditorCreator);
        pp->RegisterExtension(splinePointEditorCreator);
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

    FieldDescriptor selectionField;
    selectionField.type = ReflectedTypeDB::Get<SelectionData>();
    selectionField.fieldName = FastName(SelectionData::selectionPropertyName);
    fieldBinder->BindField(selectionField, DAVA::MakeFunction(this, &SplineEditorModule::OnSelectionChanged));

    // Delete Spline Points action
    {
        const QString actionName = "Remove Selected Points";
        QtAction* action = new QtAction(GetAccessor(), actionName);
        action->setShortcut(QKeySequence(Qt::Key_Shift + Qt::Key_R));
        action->setShortcutContext(Qt::WindowShortcut);
        action->SetStateUpdationFunction(QtAction::Enabled, selectionField, [](const Any& value)
                                         {
                                             return value.Cast<SelectableGroup>(SelectableGroup()).ContainsOnlyObjectsOfType<SplineComponent::SplinePoint>();
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&SplineEditorModule::OnDeleteSelectedPoints, this));

        KeyBindableActionInfo keyInfo;
        keyInfo.blockName = "Spline Editor";
        keyInfo.defaultShortcuts = action->shortcuts();
        MakeActionKeyBindable(action, keyInfo);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit));
        GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, action);
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

void SplineEditorModule::OnDeleteSelectedPoints()
{
    using namespace DAVA;

    DataContext* context = GetAccessor()->GetActiveContext();
    if (context == nullptr || context->GetData<SceneData>() == nullptr)
    {
        return;
    }
    SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
    SelectionSystem* selectionSystem = scene->GetSystem<SelectionSystem>();
    SplineEditorSystem* system = scene->GetSystem<SplineEditorSystem>();
    const Vector<Selectable>& selection = selectionSystem->GetSelection().GetContent();

    scene->BeginBatch("Remove Spline Points", static_cast<uint32>(selection.size()));

    for (const Selectable& selectable : selection)
    {
        SplineComponent::SplinePoint* point = selectable.Cast<SplineComponent::SplinePoint>();
        SplineComponent* spline = system->GetSplineByPoint(point);
        scene->Exec(std::make_unique<RemoveSplinePointCommand>(scene, spline, point));
    }

    scene->EndBatch();
}

DAVA_VIRTUAL_REFLECTION_IMPL(SplineEditorModule)
{
    DAVA::ReflectionRegistrator<SplineEditorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(SplineEditorModule);
