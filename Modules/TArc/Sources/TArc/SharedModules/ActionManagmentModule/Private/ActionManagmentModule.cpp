#include "TArc/SharedModules/ActionManagmentModule/ActionManagmentModule.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/Private/UIProxy.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/WindowSubSystem/ActionUtils.h"

#include "TArc/SharedModules/ActionManagmentModule/Private/ActionManagmentDialog.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
void ActionManagmentModule::PostInit()
{
    executor.DelayedExecute([this]() {
        UI* ui = GetUI();
        ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "Tools"));
        QAction* action = new QAction("Key bindings", nullptr);
        ui->AddAction(mainWindowKey, placementInfo, action);

        connections.AddConnection(action, &QAction::triggered, [this]() {
            ActionManagmentDialog dlg(GetAccessor(), static_cast<UIManager*>(static_cast<UIProxy*>(GetUI())->GetGlobalUI()));
            GetUI()->ShowModalDialog(mainWindowKey, &dlg);
        });
    });
}

DAVA_VIRTUAL_REFLECTION_IMPL(ActionManagmentModule)
{
    ReflectionRegistrator<ActionManagmentModule>::Begin()
    .ConstructorByPointer()
    .End();
}
} // namespace TArc
} // namespace DAVA
