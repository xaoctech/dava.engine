#if defined(__DAVAENGINE_BEAST__)

#include "Classes/BeastModule/BeastModule.h"

#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <Reflection/ReflectionRegistrator.h>

void BeastModule::PostInit()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    //     QtAction* action = new QtAction(GetAccessor(), QIcon(":/QtIcons/text_component.png"), QString("Text Drawing Enabled"));
    //     { // checked-unchecked and text
    //         FieldDescriptor fieldDescr;
    //         fieldDescr.fieldName = DAVA::FastName(TextModuleData::drawingEnabledPropertyName);
    //         fieldDescr.type = DAVA::ReflectedTypeDB::Get<TextModuleData>();
    //         action->SetStateUpdationFunction(QtAction::Checked, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
    //             return value.Get<bool>(false);
    //         });
    //         action->SetStateUpdationFunction(QtAction::Text, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
    //             if (value.Get<bool>(false))
    //                 return DAVA::String("Text Drawing Enabled");
    //             return DAVA::String("Text Drawing Disabled");
    //         });
    //     }
    //
    //     { // enabled/disabled state
    //         FieldDescriptor fieldDescr;
    //         fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
    //         fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
    //         action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
    //             return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
    //         });
    //     }
    //
    //     action->setShortcut(QKeySequence("Ctrl+F"));
    //     action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    //
    //     connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&TextModule::ChangeDrawingState, this));
    //
    //     ActionPlacementInfo placementInfo;
    //     placementInfo.AddPlacementPoint(CreateStatusbarPoint(true, 0, { InsertionParams::eInsertionMethod::AfterItem, "actionShowStaticOcclusion" }));
    //
    //     GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

DAVA_VIRTUAL_REFLECTION_IMPL(BeastModule)
{
    DAVA::ReflectionRegistrator<BeastModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(BeastModule);

#endif //#if defined (__DAVAENGINE_BEAST__)
