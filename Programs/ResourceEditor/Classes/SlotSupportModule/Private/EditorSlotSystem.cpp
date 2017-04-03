#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"

#include <Scene3D/Systems/SlotSystem.h>

EditorSlotSystem::EditorSlotSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void EditorSlotSystem::AddComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    entities.push_back(entity);
}

void EditorSlotSystem::RemoveComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    FindAndRemoveExchangingWithLast(entities, entity);
}

void EditorSlotSystem::Process(float32 timeElapsed)
{
    SlotSystem* slotSystem = GetScene()->slotSystem;
    for (Entity* entity : entities)
    {
        uint32 slotCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
        for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(slotCount, slotIndex));
            Entity* loadedEntity = slotSystem->LookUpLoadedEntity(component);
            if (loadedEntity == nullptr)
            {
                Entity* newEntity = new Entity();
                newEntity->SetNotRemovable(true);
                slotSystem->AttachEntityToSlot(component, newEntity);
            }
        }
    }
}
