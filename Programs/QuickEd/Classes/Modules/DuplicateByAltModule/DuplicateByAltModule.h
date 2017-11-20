#pragma once

#include "Classes/Modules/BaseEditorModule.h"

class DuplicateByAltModule : public BaseEditorModule
{
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    void OnDuplicateRequested();

    DAVA_VIRTUAL_REFLECTION(DuplicateByAltModule, DAVA::TArc::ClientModule);
};
