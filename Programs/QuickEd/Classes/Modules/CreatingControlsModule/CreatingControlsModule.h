#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtDelayedExecutor.h>

class EditorSystemsManager;

class CreatingControlsModule : public DAVA::TArc::ClientModule
{
public:
    CreatingControlsModule() = default;

private:
    // ClientModule
    void PostInit() override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void OnCreateByClick(ControlNode* control);

private:
    DAVA::TArc::QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION(CreatingControlsModule, DAVA::TArc::ClientModule);
};