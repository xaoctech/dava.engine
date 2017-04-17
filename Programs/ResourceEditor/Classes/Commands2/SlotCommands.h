#pragma once

#include "Classes/Commands2/Base/RECommand.h"
#include <Base/RefPtr.h>

class SceneEditor2;
namespace DAVA
{
class Entity;
class SlotComponent;
}

class AttachEntityToSlot : public RECommand
{
public:
    explicit AttachEntityToSlot(SceneEditor2* sceneEditor, DAVA::SlotComponent* slotComponent, DAVA::Entity* entity);
    explicit AttachEntityToSlot(SceneEditor2* sceneEditor, DAVA::SlotComponent* slotComponent, const DAVA::FastName& itemName);

    void Redo() override;
    void Undo() override;

    bool IsClean() const override;

private:
    SceneEditor2* sceneEditor = nullptr;
    DAVA::SlotComponent* component = nullptr;
    DAVA::RefPtr<DAVA::Entity> redoEntity;
    DAVA::RefPtr<DAVA::Entity> undoEntity;
    DAVA::FastName itemName;
    bool redoEntityInited = false;
};
