#pragma once

#include "Classes/Commands2/Base/RECommand.h"

class DetachEntityFromSlot : public RECommand
{
public:
    DetachEntityFromSlot(SceneEditor2* sceneEditor, DAVA::SlotComponent* slotComponent, DAVA::Entity* entity);

    void Redo() override;
    void Undo() override;

private:
    SceneEditor2* sceneEditor = nullptr;
    DAVA::SlotComponent* component = nullptr;
    DAVA::RefPtr<DAVA::Entity> entity;
};

class AttachEntityToSlot : public RECommand
{
public:
    AttachEntityToSlot(SceneEditor2* sceneEditor, DAVA::SlotComponent* slotComponent, const DAVA::FastName& itemName);

    void Redo() override;
    void Undo() override;

private:
    SceneEditor2* sceneEditor = nullptr;
    DAVA::SlotComponent* component = nullptr;
    DAVA::RefPtr<DAVA::Entity> entity = nullptr;
    DAVA::FastName itemName;
};