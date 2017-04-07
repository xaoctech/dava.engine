#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/Commands2/SlotCommands.h"

#include <Scene3D/Systems/SlotSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Scene.h>
#include <Base/BaseTypes.h>
#include <Utils/Utils.h>

EditorSlotSystem::EditorSlotSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    GetScene()->eventSystem->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
}

void EditorSlotSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED)
    {
        Entity* entity = component->GetEntity();
        SlotComponent* slotComponent = GetScene()->slotSystem->LookUpSlot(entity);
        if (slotComponent != nullptr)
        {
            slotComponent->SetAttachmentTransform(entity->GetLocalTransform());
        }
    }
}

void EditorSlotSystem::RegisterEntity(DAVA::Entity* entity)
{
    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0)
    {
        entities.push_back(entity);
        pendingOnInitialize.insert(entity);
    }
}

void EditorSlotSystem::UnregisterEntity(DAVA::Entity* entity)
{
    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0)
    {
        FindAndRemoveExchangingWithLast(entities, entity);
        pendingOnInitialize.erase(entity);
    }
}

void EditorSlotSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
    {
        pendingOnInitialize.insert(entity);
        if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) == 1)
        {
            entities.push_back(entity);
        }
    }
}

void EditorSlotSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
    {
        if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) == 1)
        {
            FindAndRemoveExchangingWithLast(entities, entity);
            pendingOnInitialize.erase(entity);
        }
    }
}

void EditorSlotSystem::Process(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    for (Entity* entity : pendingOnInitialize)
    {
        Set<FastName> names;
        Set<SlotComponent*> uninitializedSlots;
        for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
        {
            SlotComponent* slotComponent = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
            FastName slotName = slotComponent->GetSlotName();
            if (slotName.IsValid())
            {
                names.insert(slotName);
            }
            else
            {
                uninitializedSlots.insert(slotComponent);
            }
        }

        uint32 slotIndex = 1;
        for (SlotComponent* component : uninitializedSlots)
        {
            FastName newSlotName(Format("Slot_%u", slotIndex++));
            while (names.count(newSlotName) > 0)
            {
                newSlotName = FastName(Format("Slot_%u", slotIndex++));
            }

            component->SetSlotName(newSlotName);
        }
    }

    SlotSystem* slotSystem = GetScene()->slotSystem;
    for (Entity* entity : pendingOnInitialize)
    {
        uint32 slotCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
        for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, slotIndex));
            Entity* loadedEntity = slotSystem->LookUpLoadedEntity(component);
            if (loadedEntity == nullptr)
            {
                Entity* newEntity = new Entity();
                newEntity->SetName(component->GetSlotName());
                newEntity->SetNotRemovable(true);
                slotSystem->AttachEntityToSlot(component, newEntity);
            }
        }
    }

    pendingOnInitialize.clear();
}

void EditorSlotSystem::DetachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity)
{
    DAVA::Entity* slotEntity = component->GetEntity();
    DAVA::Entity* loadedEntity = GetScene()->slotSystem->LookUpLoadedEntity(component);
    DVASSERT(loadedEntity == entity);
    DVASSERT(slotEntity == entity->GetParent());

    slotEntity->RemoveNode(entity);
}

void EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity)
{
    SlotSystem* slotSystem = GetScene()->slotSystem;
    DVASSERT(slotSystem->LookUpLoadedEntity(component) == nullptr);
    slotSystem->AttachEntityToSlot(component, entity);
}

DAVA::Entity* EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, const DAVA::FastName& itemName)
{
    SlotSystem* slotSystem = GetScene()->slotSystem;
    DVASSERT(slotSystem->LookUpLoadedEntity(component) == nullptr);
    return slotSystem->AttachItemToSlot(component, itemName);
}

std::unique_ptr<DAVA::Command> EditorSlotSystem::PrepareForSave(bool /*saveForGame*/)
{
    if (entities.empty())
    {
        return nullptr;
    }

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    SlotSystem* slotSystem = sceneEditor->slotSystem;

    RECommandBatch* batchCommand = new RECommandBatch("Prepare for save", static_cast<DAVA::uint32>(entities.size()));
    for (Entity* entity : entities)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
            Entity* loadedEntity = slotSystem->LookUpLoadedEntity(component);

            batchCommand->Add(std::make_unique<DetachEntityFromSlot>(sceneEditor, component, loadedEntity));
        }
    }

    return std::unique_ptr<DAVA::Command>(batchCommand);
}
