#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Commands2/RECommandIDs.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"

#include <Scene3D/Systems/SlotSystem.h>

AttachEntityToSlot::AttachEntityToSlot(SceneEditor2* sceneEditor_, DAVA::SlotComponent* slotComponent, DAVA::Entity* entity)
    : RECommand(CMDID_ATTACH_TO_SLOT, "Add item to slot")
    , sceneEditor(sceneEditor_)
    , component(slotComponent)
    , redoEntity(DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(entity))
    , redoEntityInited(true)
{
    undoEntity = DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(sceneEditor->slotSystem->LookUpLoadedEntity(slotComponent));
}

AttachEntityToSlot::AttachEntityToSlot(SceneEditor2* sceneEditor_, DAVA::SlotComponent* slotComponent, const DAVA::FastName& itemName_)
    : RECommand(CMDID_ATTACH_TO_SLOT, "Add item to slot")
    , sceneEditor(sceneEditor_)
    , component(slotComponent)
    , itemName(itemName_)
    , redoEntityInited(false)
{
    undoEntity = DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(sceneEditor->slotSystem->LookUpLoadedEntity(slotComponent));
}

void AttachEntityToSlot::Redo()
{
    EditorSlotSystem* system = sceneEditor->LookupEditorSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);

    if (undoEntity.Get() != nullptr)
    {
        system->DetachEntity(component, undoEntity.Get());
    }

    if (redoEntityInited == false)
    {
        redoEntity = DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(system->AttachEntity(component, itemName));
        redoEntityInited = true;
    }
    else
    {
        if (redoEntity.Get() != nullptr)
        {
            system->AttachEntity(component, redoEntity.Get());
        }
    }
}

void AttachEntityToSlot::Undo()
{
    EditorSlotSystem* system = sceneEditor->LookupEditorSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);
    if (redoEntity.Get() != nullptr)
    {
        system->DetachEntity(component, redoEntity.Get());
    }

    if (undoEntity.Get() != nullptr)
    {
        system->AttachEntity(component, undoEntity.Get());
    }
}

bool AttachEntityToSlot::IsClean() const
{
    return true;
}
