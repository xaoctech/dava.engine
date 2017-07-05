#include "Classes/TextModule/TextModule.h"
#include "Classes/TextModule/Private/EditorTextSystem.h"
#include "Classes/TextModule/Private/TextModuleData.h"

#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/DataProcessing/Common.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <Reflection/ReflectionRegistrator.h>

#include <QShortcut>

void TextModule::PostInit()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    QtAction* action = new QtAction(GetAccessor(), QIcon(":/QtIcons/text_component.png"), QString("Text Drawing Enabled"));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(TextModuleData::drawingEnabledPropertyName);
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<TextModuleData>();
        action->SetStateUpdationFunction(QtAction::Checked, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            return value.Get<bool>(false);
        });
        action->SetStateUpdationFunction(QtAction::Text, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            if (value.Get<bool>(false))
                return DAVA::String("Text Drawing Enabled");
            return DAVA::String("Text Drawing Disabled");
        });
    }

    action->setShortcut(QKeySequence("Ctrl+F"));

    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&TextModule::ChangeDrawingState, this));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateStatusbarPoint(true, 0, { InsertionParams::eInsertionMethod::AfterItem, "actionShowStaticOcclusion" }));

    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

void TextModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<TextModuleData> moduleData = std::make_unique<TextModuleData>();
    moduleData->editorTextSystem.reset(new EditorTextSystem(scene));
    moduleData->editorTextSystem->EnableSystem();
    scene->AddSystem(moduleData->editorTextSystem.get(), MAKE_COMPONENT_MASK(DAVA::Component::TEXT_COMPONENT));

    context->CreateData(std::move(moduleData));
}

void TextModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    TextModuleData* moduleData = context->GetData<TextModuleData>();
    scene->RemoveSystem(moduleData->editorTextSystem.get());
}

void TextModule::ChangeDrawingState()
{
    DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    TextModuleData* moduleData = context->GetData<TextModuleData>();

    bool enabled = moduleData->IsDrawingEnabled();
    moduleData->SetDrawingEnabled(!enabled);
}

DAVA_VIRTUAL_REFLECTION_IMPL(TextModule)
{
    DAVA::ReflectionRegistrator<TextModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(TextModule);
