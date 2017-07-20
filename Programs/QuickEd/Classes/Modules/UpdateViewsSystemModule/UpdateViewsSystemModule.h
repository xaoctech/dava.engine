#pragma once

#include <TArc/Core/ClientModule.h>

class UpdateViewsSystemModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key);

    DAVA_VIRTUAL_REFLECTION(UpdateViewsSystemModule, DAVA::TArc::ClientModule);
};
