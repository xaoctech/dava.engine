#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Utils/QtDelayedExecutor.h"

namespace DAVA
{
namespace TArc
{
class SettingsManager;
class SettingsModule : public ClientModule
{
public:
    SettingsModule();
    SettingsModule(ActionPlacementInfo placementInfo, QString actionName);

private:
    void PostInit() override;
    void ShowSettings();

    ActionPlacementInfo placementInfo;
    QString actionName;
    std::unique_ptr<SettingsManager> manager;
    QtConnections connections;
    QtDelayedExecutor executor;

    DAVA_VIRTUAL_REFLECTION(SettingsModule, ClientModule);
};

void InitColorPickerOptions(bool initForHiddenUsage);
} // namespace TArc
} // namespace DAVA