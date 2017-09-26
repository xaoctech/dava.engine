#pragma once

#include "Classes/Modules/BaseEditorModule.h"

class ControlNode;

class HUDModule : public BaseEditorModule
{
private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    void OnHighlightChanged(ControlNode* node);

    DAVA_VIRTUAL_REFLECTION(HUDModule, DAVA::TArc::ClientModule);
};
