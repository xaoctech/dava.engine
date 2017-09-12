#include "Classes/ObjectPlacement/ObjectPlacementModule.h"
#include "Classes/ObjectPlacement/Private/ObjectPlacementData.h"
#include "Classes/ObjectPlacement/Private/ObjectPlacementSystem.h"

#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Selection/Selection.h"

#include <TArc/WindowSubSystem/QtAction.h>

#include <Scene3D/Components/ComponentHelpers.h>

void ObjectPlacementModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    std::unique_ptr<ObjectPlacementData> objectPlacementData =
        std::make_unique<ObjectPlacementData>();

    objectPlacementData->objectPlacementSystem.reset(new ObjectPlacementSystem(scene));
    scene->AddSystem(objectPlacementData->objectPlacementSystem.get(), MAKE_COMPONENT_MASK(DAVA::Component::RENDER_COMPONENT));

    context->CreateData(std::move(objectPlacementData));
}

void ObjectPlacementModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    ObjectPlacementData* objectPlacementData = context->GetData<ObjectPlacementData>();
    scene->RemoveSystem(objectPlacementData->objectPlacementSystem.get());
}

void ObjectPlacementModule::PostInit()
{
    const QString toolBarName("Main Toolbar");
    const QString editMenuName("Edit");

    const QString centerPivotPointName("actionCenterPivotPoint");

    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    // Place on landscape
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/modify_placeonland.png"), QString("Place on landscape"));
        { // enable/disable
            FieldDescriptor fieldDescr;
            fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
            fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
            action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
                return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
            });
        }
        action->setShortcuts({ QKeySequence(Qt::Key_P) });
        action->setShortcutContext(Qt::WindowShortcut);
        connections.AddConnection(action, &QAction::triggered, DAVA::MakeFunction(this, &ObjectPlacementModule::OnPlaceOnLandscape));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit,
                                                        { InsertionParams::eInsertionMethod::AfterItem, centerPivotPointName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(mainWindowKey, placementInfo, action);
    }

    // Snap to landscape
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/modify_snaptoland.png"), "Enable snap to landscape");
        { // check/uncheck
            FieldDescriptor fieldDescr;
            fieldDescr.fieldName = DAVA::FastName(ObjectPlacementData::snapToLandscapePropertyName);
            fieldDescr.type = DAVA::ReflectedTypeDB::Get<ObjectPlacementData>();
            action->SetStateUpdationFunction(QtAction::Checked, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
                return value.Get<bool>(false);
            });
        }

        { // enable/disable
            FieldDescriptor fieldDescr;
            fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
            fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
            action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
                return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
            });
        }

        { // tooltip text
            FieldDescriptor fieldDescr;
            fieldDescr.fieldName = DAVA::FastName(ObjectPlacementData::snapToLandscapePropertyName);
            fieldDescr.type = DAVA::ReflectedTypeDB::Get<ObjectPlacementData>();
            action->SetStateUpdationFunction(QtAction::Text, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
               bool checked = value.Get<bool>(false);
               if (checked)
                   return DAVA::String("Disable snap to landscape");
               return DAVA::String("Enable snap to landscape");
            });
        }

        connections.AddConnection(action, &QAction::triggered, DAVA::MakeFunction(this, &ObjectPlacementModule::OnSnapToLandscape));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit,
                                                        { InsertionParams::eInsertionMethod::AfterItem,  "Place on landscape"}));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(mainWindowKey, placementInfo, action);
    }

    // Place and align
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/modify_placeonobj.png"), QString("Place and align"));
        { // enable/disable
            FieldDescriptor fieldDescr;
            fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
            fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
            action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
                return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
            });
        }
        action->setShortcuts({ QKeySequence(Qt::CTRL + Qt::Key_P) });
        action->setShortcutContext(Qt::WindowShortcut);
        connections.AddConnection(action, &QAction::triggered, DAVA::MakeFunction(this, &ObjectPlacementModule::OnPlaceAndAlign));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit,
                                                        { InsertionParams::eInsertionMethod::AfterItem, "Place on landscape" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(mainWindowKey, placementInfo, action);
    }
}

void ObjectPlacementModule::OnPlaceOnLandscape()
{
    DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    ObjectPlacementData* data = context->GetData<ObjectPlacementData>();
    data->objectPlacementSystem->PlaceOnLandscape();
}

void ObjectPlacementModule::OnSnapToLandscape()
{
    DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    ObjectPlacementData* data = context->GetData<ObjectPlacementData>();
    bool snapToLandscapeEnabled = data->GetSnapToLandscape();
    data->SetSnapToLandscape(!snapToLandscapeEnabled);
}

void ObjectPlacementModule::OnPlaceAndAlign()
{
    DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    ObjectPlacementData* data = context->GetData<ObjectPlacementData>();
    data->objectPlacementSystem->PlaceAndAlign();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ObjectPlacementModule)
{
    DAVA::ReflectionRegistrator<ObjectPlacementModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(ObjectPlacementModule);
