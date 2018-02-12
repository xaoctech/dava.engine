#pragma once

#include <TArc/Core/ClientModule.h>

class LightingPresetsModule : public DAVA::ClientModule
{
protected:
    void PostInit() override;

private:
    void ImportLightingPreset();
    void ExportLightingPreset();

    DAVA_VIRTUAL_REFLECTION(LightingPresetsModule, DAVA::ClientModule);
};