#pragma once

#include <TArc/Core/ClientModule.h>

class InputModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    DAVA_VIRTUAL_REFLECTION(InputModule, DAVA::TArc::ClientModule);
};
