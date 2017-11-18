#pragma once

#include <TArc/Core/ClientModule.h>

class PreferencesModule : public DAVA::TArc::ClientModule
{
public:
    PreferencesModule();

private:
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(PreferencesModule, DAVA::TArc::ClientModule);
};
