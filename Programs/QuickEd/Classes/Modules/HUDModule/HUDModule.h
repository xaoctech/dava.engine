#pragma once

#include "Classes/Modules/QEClientModule.h"

class ControlNode;

class HUDModule : public QEClientModule
{
private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    void OnHighlightChanged(ControlNode* node);

    DAVA_VIRTUAL_REFLECTION(HUDModule, DAVA::TArc::ClientModule);
};
