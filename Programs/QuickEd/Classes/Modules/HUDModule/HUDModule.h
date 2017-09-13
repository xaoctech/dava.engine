#pragma once

#include <TArc/Core/ClientModule.h>

class ControlNode;

class HUDModule : public DAVA::TArc::ClientModule
{
private:
    void PostInit() override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void OnHighlightChanged(ControlNode* node);

    DAVA_VIRTUAL_REFLECTION(HUDModule, DAVA::TArc::ClientModule);
};
