#pragma once

#include <TArc/Core/ClientModule.h>

class PixelGridModule : public DAVA::TArc::ClientModule
{
public:
    PixelGridModule();

private:
    void PostInit() override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    DAVA_VIRTUAL_REFLECTION(PixelGridModule, DAVA::TArc::ClientModule);
};
