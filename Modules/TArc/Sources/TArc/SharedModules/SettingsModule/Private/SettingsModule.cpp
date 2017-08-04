#include "TArc/SharedModules/SettingsModule/SettingsModule.h"
#include "TArc/SharedModules/SettingsModule/Private/SettingsManager.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/Controls/ColorPicker/ColorPickerSettings.h"
#include "TArc/Qt/QtString.h"
#include "TArc/Qt/QtIcon.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Functional/Function.h>

#include <QList>
#include "SettingsDialog.h"

namespace DAVA
{
namespace TArc
{
namespace SettingsModuleDetails
{
class SettingsContainerNode : public DataNode
{
public:
    Reflection settingsContainer;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SettingsContainerNode, DataNode)
    {
        ReflectionRegistrator<SettingsContainerNode>::Begin()
        .Field("container", &SettingsContainerNode::settingsContainer)
        .End();
    }
};
}
SettingsModule::SettingsModule()
{
    placementInfo.AddPlacementPoint(CreateMenuPoint(QList<QString>() << "Tools"));
    actionName = "Settings";
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ColorPickerSettings);
}

SettingsModule::SettingsModule(const ActionPlacementInfo& placementInfo_, const QString& actionName_)
    : placementInfo(placementInfo_)
    , actionName(actionName_)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ColorPickerSettings);
}

void SettingsModule::PostInit()
{
    ContextAccessor* accessor = GetAccessor();

    manager.reset(new SettingsManager(GetAccessor()));
    manager->CreateSettings();

    SettingsModuleDetails::SettingsContainerNode* node = new SettingsModuleDetails::SettingsContainerNode();
    node->settingsContainer = Reflection::Create(ReflectedObject(manager->settings.get()));
    accessor->GetGlobalContext()->CreateData(std::unique_ptr<DataNode>(node));

    executor.DelayedExecute([this]() {
        QtAction* settingsAction = new QtAction(GetAccessor(), QIcon(":/QtIcons/settings.png"), actionName);

        GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, settingsAction);
        connections.AddConnection(settingsAction, &QAction::triggered, DAVA::MakeFunction(this, &SettingsModule::ShowSettings));
    });
}

void SettingsModule::ShowSettings()
{
    SettingsDialog::Params params;
    params.accessor = GetAccessor();
    params.ui = GetUI();
    params.objectsField.type = ReflectedTypeDB::Get<SettingsModuleDetails::SettingsContainerNode>();
    params.objectsField.fieldName = FastName("container");
    SettingsDialog settingsDialog(params);
    settingsDialog.resetSettings.Connect([this]() {
        manager->ResetToDefault();
    });

    GetUI()->ShowModalDialog(DAVA::TArc::mainWindowKey, &settingsDialog);
}

DAVA_VIRTUAL_REFLECTION_IMPL(SettingsModule)
{
    ReflectionRegistrator<SettingsModule>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const ActionPlacementInfo&, const QString&>()
    .End();
}

} // namespace TArc
} // namespace DAVA