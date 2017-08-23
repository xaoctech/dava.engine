#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/SlotSupportModule/Private/SlotTemplatesData.h"
#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Commands2/Base/RECommandBatch.h"
#include "Classes/Commands2/SetFieldValueCommand.h"
#include "Classes/Commands2/RemoveComponentCommand.h"
#include "Classes/Commands2/AddComponentCommand.h"
#include "Classes/Commands2/EntityRemoveCommand.h"
#include "Classes/Commands2/Base/RECommandNotificationObject.h"
#include "Classes/Commands2/EntityAddCommand.h"
#include "Classes/Selection/Selection.h"
#include "Classes/Qt/Scene/SceneSignals.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Scene3D/Systems/SlotSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Scene.h>
#include <Render/RenderHelper.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Base/BaseTypes.h>

#include <Utils/Utils.h>
#include <QObject>

const DAVA::FastName EditorSlotSystem::emptyItemName = DAVA::FastName("Empty");

namespace EditorSlotSystemDetail
{
void DetachSlotForRemovingEntity(DAVA::Entity* entity, SceneEditor2* scene, REDependentCommandsHolder& holder)
{
    for (DAVA::uint32 i = 0; i < entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT); ++i)
    {
        DAVA::SlotComponent* component = static_cast<DAVA::SlotComponent*>(entity->GetComponent(DAVA::Component::SLOT_COMPONENT, i));
        holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, component, nullptr, DAVA::FastName()));
    }

    for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        DetachSlotForRemovingEntity(entity->GetChild(i), scene, holder);
    }
}
} // namespace EditorSlotSystemDetail

EditorSlotSystem::EditorSlotSystem(DAVA::Scene* scene, DAVA::TArc::ContextAccessor* accessor_)
    : SceneSystem(scene)
    , accessor(accessor_)
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

    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    SlotSystem* slotSystem = scene->slotSystem;
    if (scene->modifSystem->InCloneState() == false &&
        scene->modifSystem->InCloneDoneState() == false)
    {
        for (Entity* entity : pendingOnInitialize)
        {
            uint32 slotCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
            for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
            {
                SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, slotIndex));
                Entity* loadedEntity = slotSystem->LookUpLoadedEntity(component);
                if (loadedEntity == nullptr)
                {
                    FastName itemNameInitialAttach;

                    Vector<SlotSystem::ItemsCache::Item> items = slotSystem->GetItems(component->GetConfigFilePath());
                    FastName templateName = component->GetTemplateName();
                    if (templateName.IsValid())
                    {
                        auto iter = std::find_if(items.begin(), items.end(), [templateName](const SlotSystem::ItemsCache::Item& item) {
                            return item.type == templateName;
                        });

                        if (iter != items.end())
                        {
                            itemNameInitialAttach = iter->itemName;
                        }
                    }
                    if (itemNameInitialAttach.IsValid() == false)
                    {
                        RefPtr<Entity> newEntity(new Entity());
                        slotSystem->AttachEntityToSlot(component, newEntity.Get(), emptyItemName);
                    }
                    else
                    {
                        slotSystem->AttachItemToSlot(component, itemNameInitialAttach);
                    }
                }
                else
                {
                    loadedEntity->SetName(component->GetSlotName());
                }
            }
        }

        pendingOnInitialize.clear();
    }

    for (Entity* entity : scene->transformSingleComponent->localTransformChanged)
    {
        SlotComponent* slot = scene->slotSystem->LookUpSlot(entity);
        if (slot == nullptr)
        {
            continue;
        }

        Matrix4 jointTranfsorm = scene->slotSystem->GetJointTransform(slot);
        bool inverseSuccessed = jointTranfsorm.Inverse();
        DVASSERT(inverseSuccessed);
        Matrix4 attachmentTransform = entity->GetLocalTransform() * jointTranfsorm;
        scene->slotSystem->SetAttachmentTransform(slot, attachmentTransform);
    }

    for (Entity* entity : entities)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
            Entity* loadedEntity = scene->slotSystem->LookUpLoadedEntity(component);
            if (loadedEntity != nullptr)
            {
                for (int32 j = 0; j < loadedEntity->GetChildrenCount(); ++j)
                {
                    Entity* child = loadedEntity->GetChild(j);
                    if (child->GetLocked() == false)
                    {
                        child->SetLocked(true);
                    }
                }
            }
        }
    }
}

void EditorSlotSystem::WillClone(DAVA::Entity* originalEntity)
{
    auto extractSlots = [this](DAVA::Entity* entity)
    {
        DAVA::uint32 slotCount = entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT);
        if (slotCount > 0)
        {
            DAVA::Scene* scene = GetScene();
            for (DAVA::uint32 i = 0; i < slotCount; ++i)
            {
                AttachedItemInfo info;
                info.component = static_cast<DAVA::SlotComponent*>(entity->GetComponent(DAVA::Component::SLOT_COMPONENT, i));
                info.entity = DAVA::RefPtr<DAVA::Entity>::ConstructWithRetain(scene->slotSystem->LookUpLoadedEntity(info.component));
                info.itemName = info.component->GetLoadedItemName();

                inClonedState[entity].push_back(info);
                DetachEntity(info.component, info.entity.Get());
            }
        }
    };

    extractSlots(originalEntity);
    DAVA::Vector<DAVA::Entity*> children;
    originalEntity->GetChildEntitiesWithComponent(children, DAVA::Component::SLOT_COMPONENT);
    for (DAVA::Entity* e : children)
    {
        extractSlots(e);
    }
}

void EditorSlotSystem::DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity)
{
    auto restoreSlots = [this](DAVA::Entity* entity)
    {
        auto iter = inClonedState.find(entity);
        if (iter == inClonedState.end())
        {
            return;
        }

        const DAVA::Vector<AttachedItemInfo> infos = iter->second;
        for (const AttachedItemInfo& info : infos)
        {
            AttachEntity(info.component, info.entity.Get(), info.itemName);
        }
        inClonedState.erase(iter);
    };

    restoreSlots(originalEntity);
    DAVA::Vector<DAVA::Entity*> children;
    originalEntity->GetChildEntitiesWithComponent(children, DAVA::Component::SLOT_COMPONENT);
    for (DAVA::Entity* e : children)
    {
        restoreSlots(e);
    }

    clonedEntityes.insert(newEntity);
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

DAVA::RefPtr<DAVA::Entity> EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, DAVA::FastName itemName)
{
    Selection::Lock();
    SCOPE_EXIT
    {
        Selection::Unlock();
    };

    DAVA::SlotSystem* slotSystem = GetScene()->slotSystem;
    if (itemName == emptyItemName)
    {
        DAVA::RefPtr<DAVA::Entity> newEntity(new DAVA::Entity());
        slotSystem->AttachEntityToSlot(component, newEntity.Get(), itemName);
        return newEntity;
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

    auto removeEntityVisitor = [&](const RECommand* command)
    {
        const EntityRemoveCommand* cmd = static_cast<const EntityRemoveCommand*>(command);
        DAVA::Entity* entityToRemove = cmd->GetEntity();
        EditorSlotSystemDetail::DetachSlotForRemovingEntity(entityToRemove, scene, holder);
    };

    commandInfo.ForEach(removeEntityVisitor, CMDID_ENTITY_REMOVE);

    auto loadDefaultItem = [&](DAVA::SlotComponent* component)
    {
        DAVA::Vector<DAVA::SlotSystem::ItemsCache::Item> items = scene->slotSystem->GetItems(component->GetConfigFilePath());
        if (items.empty())
        {
            DAVA::RefPtr<DAVA::Entity> newEntity(new DAVA::Entity());
            holder.AddPostCommand(std::unique_ptr<DAVA::Command>(new AttachEntityToSlot(scene, component, newEntity.Get(), emptyItemName)));
        }
        else
        {
            holder.AddPostCommand(std::unique_ptr<DAVA::Command>(new AttachEntityToSlot(scene, component, items.front().itemName)));
        }
    };

    auto addSlotVisitor = [&](const RECommand* command)
    {
        const AddComponentCommand* cmd = static_cast<const AddComponentCommand*>(command);
        DAVA::Component* component = cmd->GetComponent();
        if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
        {
            loadDefaultItem(static_cast<DAVA::SlotComponent*>(component));
        }
    };

    commandInfo.ForEach(addSlotVisitor, CMDID_COMPONENT_ADD);

    auto addEntityVisitor = [&](const RECommand* command)
    {
        const EntityAddCommand* cmd = static_cast<const EntityAddCommand*>(command);
        DAVA::Entity* entityForAdd = cmd->GetEntity();

        auto iter = clonedEntityes.find(entityForAdd);
        if (iter != clonedEntityes.end())
        {
            DAVA::Vector<DAVA::Entity*> slotEntityes;
            entityForAdd->GetChildEntitiesWithComponent(slotEntityes, DAVA::Component::SLOT_COMPONENT);
            slotEntityes.push_back(entityForAdd);

            for (DAVA::Entity* e : slotEntityes)
            {
                for (DAVA::uint32 i = 0; i < e->GetComponentCount(DAVA::Component::SLOT_COMPONENT); ++i)
                {
                    loadDefaultItem(static_cast<DAVA::SlotComponent*>(e->GetComponent(DAVA::Component::SLOT_COMPONENT, i)));
                }
            }

            clonedEntityes.erase(iter);
        }
    };

    commandInfo.ForEach(addEntityVisitor, CMDID_ENTITY_ADD);
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

void EditorSlotSystem::Draw()
{
    using namespace DAVA;
    SlotTemplatesData* data = accessor->GetGlobalContext()->GetData<SlotTemplatesData>();
    Scene* scene = GetScene();

    Color boxColor = data->GetBoxColor();
    Color boxEdgeColor = data->GetBoxEdgesColor();
    Color pivotColor = data->GetPivotColor();

    RenderHelper* rh = scene->GetRenderSystem()->GetDebugDrawer();
    for (Entity* entity : entities)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
            FastName templateName = component->GetTemplateName();
            const SlotTemplatesData::Template* t = data->GetTemplate(templateName);
            if (t != nullptr)
            {
                Vector3 min(-t->pivot.x, -t->pivot.y, -t->pivot.z);
                Vector3 max(t->boundingBoxSize.x - t->pivot.x,
                            t->boundingBoxSize.y - t->pivot.y,
                            t->boundingBoxSize.z - t->pivot.z);
                AABBox3 box(min, max);
                Entity* loadedEntity = scene->slotSystem->LookUpLoadedEntity(component);
                if (loadedEntity != nullptr)
                {
                    Matrix4 transform = loadedEntity->GetWorldTransform();
                    rh->DrawAABoxTransformed(box, transform, boxColor, RenderHelper::DRAW_SOLID_DEPTH);
                    rh->DrawAABoxTransformed(box, transform, boxEdgeColor, RenderHelper::DRAW_WIRE_DEPTH);

                    Vector3 pivot = Vector3(0.0f, 0.0f, 0.0f) * transform;
                    rh->DrawIcosahedron(pivot, 0.3f, pivotColor, RenderHelper::DRAW_SOLID_DEPTH);
                }
            }
        }
    }
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

void EditorSlotSystem::SetScene(DAVA::Scene* scene)
{
    {
        SceneEditor2* currentScene = static_cast<SceneEditor2*>(GetScene());
        if (currentScene != nullptr)
        {
            currentScene->modifSystem->RemoveDelegate(this);
        }
    }

    SceneSystem::SetScene(scene);

    {
        SceneEditor2* currentScene = static_cast<SceneEditor2*>(GetScene());
        if (currentScene != nullptr)
        {
            currentScene->modifSystem->AddDelegate(this);
        }
    }
}
