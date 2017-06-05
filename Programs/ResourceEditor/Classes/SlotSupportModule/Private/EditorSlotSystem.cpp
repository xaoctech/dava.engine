#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Commands2/Base/RECommandBatch.h"
#include "Classes/Commands2/SetFieldValueCommand.h"
#include "Classes/Commands2/RemoveComponentCommand.h"
#include "Classes/Commands2/Base/RECommandNotificationObject.h"
#include "Classes/Selection/Selection.h"
#include "Classes/Qt/Scene/SceneSignals.h"

#include <Scene3D/Systems/SlotSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Scene.h>
#include <Base/BaseTypes.h>

#include <Utils/Utils.h>
#include <QObject>

const DAVA::FastName EditorSlotSystem::emptyItemName = DAVA::FastName("Empty");

EditorSlotSystem::EditorSlotSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void EditorSlotSystem::AddEntity(DAVA::Entity* entity)
{
    DVASSERT(entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0);
    entities.push_back(entity);
    pendingOnInitialize.insert(entity);
}

void EditorSlotSystem::RemoveEntity(DAVA::Entity* entity)
{
    DVASSERT(entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0);
    DAVA::FindAndRemoveExchangingWithLast(entities, entity);
    pendingOnInitialize.erase(entity);
}

void EditorSlotSystem::AddComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    DVASSERT(component->GetType() == DAVA::Component::SLOT_COMPONENT);
    pendingOnInitialize.insert(entity);
    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) == 1)
    {
#if defined(__DAVAENGINE_DEBUG__)
        DVASSERT(std::find(entities.begin(), entities.end(), entity) == entities.end());
#endif
        entities.push_back(entity);
    }
}

void EditorSlotSystem::RemoveComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    DVASSERT(component->GetType() == DAVA::Component::SLOT_COMPONENT);
    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) == 1)
    {
        DAVA::FindAndRemoveExchangingWithLast(entities, entity);
        pendingOnInitialize.erase(entity);
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

    Scene* scene = GetScene();
    SlotSystem* slotSystem = scene->slotSystem;
    for (Entity* entity : pendingOnInitialize)
    {
        uint32 slotCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
        for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, slotIndex));
            Entity* loadedEntity = slotSystem->LookUpLoadedEntity(component);
            if (loadedEntity == nullptr)
            {
                Vector<SlotSystem::ItemsCache::Item> items = slotSystem->GetItems(component->GetConfigFilePath());
                if (items.empty())
                {
                    DAVA::RefPtr<Entity> newEntity(new Entity());
                    slotSystem->AttachEntityToSlot(component, newEntity.Get(), emptyItemName);
                }
                else
                {
                    slotSystem->AttachItemToSlot(component, items.front().itemName);
                }
            }
        }
    }

    pendingOnInitialize.clear();

    for (DAVA::Entity* entity : scene->transformSingleComponent->localTransformChanged)
    {
        SlotComponent* slot = scene->slotSystem->LookUpSlot(entity);
        if (slot == nullptr)
        {
            continue;
        }

        DAVA::Matrix4 jointTranfsorm = scene->slotSystem->GetJointTransform(slot);
        bool inverseSuccessed = jointTranfsorm.Inverse();
        DVASSERT(inverseSuccessed);
        DAVA::Matrix4 attachmentTransform = jointTranfsorm * entity->GetLocalTransform();
        scene->slotSystem->SetAttachmentTransform(slot, attachmentTransform);
    }
}

void EditorSlotSystem::DetachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity)
{
    DAVA::Entity* slotEntity = component->GetEntity();
    DAVA::Entity* loadedEntity = GetScene()->slotSystem->LookUpLoadedEntity(component);
    DVASSERT(loadedEntity == entity);
    DVASSERT(slotEntity == entity->GetParent());

    slotEntity->RemoveNode(entity);
}

void EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity, DAVA::FastName itemName)
{
    DAVA::SlotSystem* slotSystem = GetScene()->slotSystem;
    Selection::Lock();
    slotSystem->AttachEntityToSlot(component, entity, itemName);
    Selection::Unlock();
}

DAVA::Entity* EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, DAVA::FastName itemName)
{
    Selection::Lock();
    SCOPE_EXIT
    {
        Selection::Unlock();
    };

    DAVA::SlotSystem* slotSystem = GetScene()->slotSystem;
    DAVA::Entity* result = nullptr;
    if (itemName == emptyItemName)
    {
        DAVA::RefPtr<DAVA::Entity> newEntity(new DAVA::Entity());
        slotSystem->AttachEntityToSlot(component, newEntity.Get(), itemName);
        return newEntity.Get();
    }
    else
    {
        return slotSystem->AttachItemToSlot(component, itemName);
    }
}

void EditorSlotSystem::AccumulateDependentCommands(REDependentCommandsHolder& holder)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    auto changeSlotVisitor = [&](const RECommand* command)
    {
        const SetFieldValueCommand* cmd = static_cast<const SetFieldValueCommand*>(command);
        const DAVA::Reflection::Field& field = cmd->GetField();
        DAVA::ReflectedObject object = field.ref.GetDirectObject();
        DAVA::FastName fieldName = field.key.Cast<DAVA::FastName>(DAVA::FastName(""));
        const DAVA::ReflectedType* type = object.GetReflectedType();
        if (type == DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>())
        {
            DAVA::SlotComponent* component = object.GetPtr<DAVA::SlotComponent>();
            DAVA::Entity* entity = scene->slotSystem->LookUpLoadedEntity(component);
            if (fieldName == DAVA::SlotComponent::ConfigPathFieldName)
            {
                DAVA::RefPtr<DAVA::Entity> newEntity(new DAVA::Entity());
                holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, component, newEntity.Get(), emptyItemName));
            }
        }
    };

    const RECommandNotificationObject& commandInfo = holder.GetMasterCommandInfo();
    commandInfo.ForEach(changeSlotVisitor, CMDID_REFLECTED_FIELD_MODIFY);

    auto removeSlotVisitor = [&](const RECommand* command)
    {
        const RemoveComponentCommand* cmd = static_cast<const RemoveComponentCommand*>(command);
        DAVA::Component* component = const_cast<DAVA::Component*>(cmd->GetComponent());
        if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
        {
            DAVA::SlotComponent* slotComponent = static_cast<DAVA::SlotComponent*>(component);
            holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, slotComponent, nullptr, DAVA::FastName()));
        }
    };

    commandInfo.ForEach(removeSlotVisitor, CMDID_COMPONENT_REMOVE);
}

void EditorSlotSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    auto visitor = [&](const RECommand* command)
    {
        const SetFieldValueCommand* cmd = static_cast<const SetFieldValueCommand*>(command);
        const DAVA::Reflection::Field& field = cmd->GetField();
        DAVA::ReflectedObject object = field.ref.GetDirectObject();
        DAVA::FastName fieldName = field.key.Cast<DAVA::FastName>(DAVA::FastName(""));
        const DAVA::ReflectedType* type = object.GetReflectedType();
        if (type == DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>())
        {
            DAVA::SlotComponent* component = object.GetPtr<DAVA::SlotComponent>();
            if (fieldName == DAVA::SlotComponent::SlotNameFieldName)
            {
                DAVA::Entity* entity = scene->slotSystem->LookUpLoadedEntity(component);
                DVASSERT(entity != nullptr);
                entity->SetName(component->GetSlotName());
            }
        }

        if (type == DAVA::ReflectedTypeDB::Get<DAVA::Entity>() && fieldName == DAVA::Entity::EntityNameFieldName)
        {
            DAVA::Entity* entity = object.GetPtr<DAVA::Entity>();
            DAVA::SlotComponent* component = scene->slotSystem->LookUpSlot(entity);
            if (component != nullptr)
            {
                component->SetSlotName(entity->GetName());
            }
        }
    };

    commandNotification.ForEach(visitor, CMDID_REFLECTED_FIELD_MODIFY);
}

std::unique_ptr<DAVA::Command> EditorSlotSystem::PrepareForSave(bool /*saveForGame*/)
{
    if (entities.empty())
    {
        return nullptr;
    }

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    DAVA::SlotSystem* slotSystem = sceneEditor->slotSystem;

    RECommandBatch* batchCommand = new RECommandBatch("Prepare for save", static_cast<DAVA::uint32>(entities.size()));
    for (DAVA::Entity* entity : entities)
    {
        for (DAVA::uint32 i = 0; i < entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT); ++i)
        {
            DAVA::SlotComponent* component = static_cast<DAVA::SlotComponent*>(entity->GetComponent(DAVA::Component::SLOT_COMPONENT, i));
            batchCommand->Add(std::make_unique<AttachEntityToSlot>(sceneEditor, component, nullptr, DAVA::FastName()));
        }
    }

    return std::unique_ptr<DAVA::Command>(batchCommand);
}
