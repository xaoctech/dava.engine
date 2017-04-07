#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Commands2/RECommandIDs.h"

#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"

DetachEntityFromSlot::DetachEntityFromSlot(SceneEditor2* sceneEditor_, DAVA::SlotComponent* slotComponent, DAVA::Entity* entity_)
    : RECommand(CMDID_DETACH_FROM_SLOT, "Remove item from slot")
    , sceneEditor(sceneEditor_)
    , component(slotComponent)
    , entity(DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(entity_))
{
}

void DetachEntityFromSlot::Redo()
{
    EditorSlotSystem* system = sceneEditor->LookupEditorSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);
    system->DetachEntity(component, entity.Get());
}

void DetachEntityFromSlot::Undo()
{
    EditorSlotSystem* system = sceneEditor->LookupEditorSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);
    system->AttachEntity(component, entity.Get());
}

AttachEntityToSlot::AttachEntityToSlot(SceneEditor2* sceneEditor_, DAVA::SlotComponent* slotComponent, const DAVA::FastName& itemName_)
    : RECommand(CMDID_ATTACH_TO_SLOT, "Add item to slot")
    , sceneEditor(sceneEditor_)
    , component(slotComponent)
    , itemName(itemName_)
{
}

void AttachEntityToSlot::Redo()
{
    EditorSlotSystem* system = sceneEditor->LookupEditorSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);

    if (entity.Get() == nullptr)
    {
        entity = DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(system->AttachEntity(component, itemName));
    }
    else
    {
        system->AttachEntity(component, entity.Get());
    }
}

void AttachEntityToSlot::Undo()
{
    EditorSlotSystem* system = sceneEditor->LookupEditorSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);
    system->DetachEntity(component, entity.Get());
}
