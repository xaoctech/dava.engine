#pragma once

#include "Classes/Modules/QEClientModule.h"

class PixelGridModule : public QEClientModule
{
public:
    PixelGridModule();

private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    DAVA_VIRTUAL_REFLECTION(PixelGridModule, DAVA::TArc::ClientModule);
};
