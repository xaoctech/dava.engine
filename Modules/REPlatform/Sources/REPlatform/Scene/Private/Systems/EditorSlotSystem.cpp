#include "REPlatform/Scene/Systems/EditorSlotSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/DataNodes/SlotTemplatesData.h"
#include "REPlatform/DataNodes/Settings/SlotSystemSettings.h"

#include "REPlatform/Commands/RECommandBatch.h"
#include "REPlatform/Commands/SlotCommands.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Commands/RemoveComponentCommand.h"
#include "REPlatform/Commands/AddComponentCommand.h"
#include "REPlatform/Commands/EntityRemoveCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/EntityAddCommand.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Base/BaseTypes.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/RenderHelper.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Scene.h>
#include <Scene3D/SceneFile/SerializationContext.h>
#include <Scene3D/SceneFile/VersionInfo.h>
#include <Scene3D/Systems/SlotSystem.h>
#include <Utils/Utils.h>

#include <QObject>

namespace DAVA
{
const FastName EditorSlotSystem::emptyItemName = FastName("Empty");

namespace EditorSlotSystemDetail
{
void DetachSlotForRemovingEntity(Entity* entity, SceneEditor2* scene, REDependentCommandsHolder& holder)
{
    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        DetachSlotForRemovingEntity(entity->GetChild(i), scene, holder);
    }

    for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
    {
        SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
        holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, component, nullptr, FastName()));
    }
}
} // namespace EditorSlotSystemDetail

EditorSlotSystem::EditorSlotSystem(Scene* scene, ContextAccessor* accessor_)
    : SceneSystem(scene)
    , accessor(accessor_)
{
}

void EditorSlotSystem::AddEntity(Entity* entity)
{
    DVASSERT(entity->GetComponentCount(Component::SLOT_COMPONENT) > 0);
    entities.push_back(entity);
    pendingOnInitialize.insert(entity);
}

void EditorSlotSystem::RemoveEntity(Entity* entity)
{
    DVASSERT(entity->GetComponentCount(Component::SLOT_COMPONENT) > 0);
    FindAndRemoveExchangingWithLast(entities, entity);
    pendingOnInitialize.erase(entity);
}

void EditorSlotSystem::AddComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::SLOT_COMPONENT);
    pendingOnInitialize.insert(entity);
    if (entity->GetComponentCount(Component::SLOT_COMPONENT) == 1)
    {
#if defined(__DAVAENGINE_DEBUG__)
        DVASSERT(std::find(entities.begin(), entities.end(), entity) == entities.end());
#endif
        entities.push_back(entity);
    }
}

void EditorSlotSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::SLOT_COMPONENT);
    if (entity->GetComponentCount(Component::SLOT_COMPONENT) == 1)
    {
        FindAndRemoveExchangingWithLast(entities, entity);
        pendingOnInitialize.erase(entity);
    }
}

void EditorSlotSystem::PrepareForRemove()
{
    entities.clear();
    pendingOnInitialize.clear();
    DVASSERT(clonedEntityes.empty());
    DVASSERT(inClonedState.empty());
}

void EditorSlotSystem::Process(float32 timeElapsed)
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
    EntityModificationSystem* modifSystem = scene->GetSystem<EntityModificationSystem>();
    SlotSystem* slotSystem = scene->slotSystem;

    if (modifSystem->InCloneState() == false && modifSystem->InCloneDoneState() == false)
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
                    FastName itemName = GetSuitableItemName(component);
                    if (itemName.IsValid() == false)
                    {
                        RefPtr<Entity> newEntity(new Entity());
                        slotSystem->AttachEntityToSlot(component, newEntity.Get(), emptyItemName);
                    }
                    else
                    {
                        slotSystem->AttachItemToSlot(component, itemName);
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

void EditorSlotSystem::WillClone(Entity* originalEntity)
{
    auto extractSlots = [this](Entity* entity)
    {
        uint32 slotCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
        if (slotCount > 0)
        {
            Scene* scene = GetScene();
            for (uint32 i = 0; i < slotCount; ++i)
            {
                AttachedItemInfo info;
                info.component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
                DVASSERT(info.component->GetEntity() != nullptr);
                info.entity = RefPtr<Entity>::ConstructWithRetain(scene->slotSystem->LookUpLoadedEntity(info.component));
                info.itemName = info.component->GetLoadedItemName();

                inClonedState[entity].push_back(info);
                DetachEntity(info.component, info.entity.Get());
            }
        }
    };

    extractSlots(originalEntity);
    Vector<Entity*> children;
    originalEntity->GetChildEntitiesWithComponent(children, Component::SLOT_COMPONENT);
    for (Entity* e : children)
    {
        extractSlots(e);
    }
}

void EditorSlotSystem::DidCloned(Entity* originalEntity, Entity* newEntity)
{
    auto restoreSlots = [this](Entity* entity)
    {
        auto iter = inClonedState.find(entity);
        if (iter == inClonedState.end())
        {
            return;
        }

        const Vector<AttachedItemInfo> infos = iter->second;
        for (const AttachedItemInfo& info : infos)
        {
            AttachEntity(info.component, info.entity.Get(), info.itemName);
        }
        inClonedState.erase(iter);
    };

    restoreSlots(originalEntity);
    Vector<Entity*> children;
    originalEntity->GetChildEntitiesWithComponent(children, Component::SLOT_COMPONENT);
    for (Entity* e : children)
    {
        restoreSlots(e);
    }

    clonedEntityes.insert(newEntity);
}

RefPtr<KeyedArchive> EditorSlotSystem::SaveSlotsPreset(Entity* entity)
{
    RefPtr<KeyedArchive> result;
    Vector<RefPtr<KeyedArchive>> archives;
    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        Entity* child = entity->GetChild(i);
        RefPtr<KeyedArchive> subEntityResult = SaveSlotsPreset(entity->GetChild(i));
        if (subEntityResult.Get() != nullptr)
        {
            archives.emplace_back(subEntityResult);
        }
    }

    if (archives.empty() == false)
    {
        result = RefPtr<KeyedArchive>(new KeyedArchive());
        uint32 subEntitiesCount = static_cast<uint32>(archives.size());
        result->SetUInt32("subEntitiesCount", subEntitiesCount);
        for (uint32 subEntityIndex = 0; subEntityIndex < subEntitiesCount; ++subEntityIndex)
        {
            RefPtr<KeyedArchive> arch = archives[subEntityIndex];
            result->SetArchive(Format("subEntity_%u", subEntityIndex), arch.Get());
        }
    }

    uint32 slotsCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
    if (slotsCount != 0)
    {
        FilePath scenePath = static_cast<SceneEditor2*>(GetScene())->GetScenePath();

        SerializationContext serializeCtx;
        serializeCtx.SetVersion(VersionInfo::Instance()->GetCurrentVersion().version);
        serializeCtx.SetScenePath(scenePath.GetDirectory());
        serializeCtx.SetRootNodePath(scenePath);
        serializeCtx.SetScene(GetScene());

        if (result.Get() == nullptr)
        {
            result = RefPtr<KeyedArchive>(new KeyedArchive());
        }

        result->SetUInt32("slotsCount", slotsCount);
        for (uint32 slotIndex = 0; slotIndex < slotsCount; ++slotIndex)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, slotIndex));
            RefPtr<KeyedArchive> arch(new KeyedArchive());
            component->Serialize(arch.Get(), &serializeCtx);
            DVASSERT(component->GetSlotName().IsValid());
            arch->SetFastName("slotName", component->GetSlotName());

            FastName loadedItem = component->GetLoadedItemName();
            if (loadedItem.IsValid())
            {
                arch->SetFastName("loadedItem", loadedItem);
            }

            result->SetArchive(Format("slot_%u", slotIndex), arch.Get());
        }
    }

    if (result.Get() != nullptr)
    {
        DVASSERT(entity->GetName().IsValid() == true);
        result->SetFastName("entityName", entity->GetName());
    }

    return result;
}

void EditorSlotSystem::LoadSlotsPreset(Entity* entity, RefPtr<KeyedArchive> archive)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    scene->BeginBatch("Load slots preset");
    FastName rootEntityName = archive->GetFastName("entityName");
    if (entity->GetName() != rootEntityName)
    {
        Reflection ref = Reflection::Create(ReflectedObject(entity));
        DVASSERT(ref.IsValid());
        Reflection::Field f;
        f.key = Entity::EntityNameFieldName;
        f.ref = ref.GetField(f.key);
        scene->Exec(std::make_unique<SetFieldValueCommand>(f, rootEntityName));
    }
    LoadSlotsPresetImpl(entity, archive);
    scene->EndBatch();
}

FastName EditorSlotSystem::GenerateUniqueSlotName(SlotComponent* component)
{
    DVASSERT(component->GetEntity() != nullptr);
    return GenerateUniqueSlotName(component, component->GetEntity(), FastName(), FastName(), Set<FastName>());
}

FastName EditorSlotSystem::GenerateUniqueSlotName(SlotComponent* component, Entity* entity,
                                                  const FastName& newTemplateName, const FastName& newEntityName,
                                                  const Set<FastName>& reservedName)
{
    FastName entityFName = newEntityName.IsValid() == true ? newEntityName : entity->GetName();
    FastName templateFName = newTemplateName.IsValid() == true ? newTemplateName : component->GetTemplateName();

    String entityName = entityFName.IsValid() == true ? entityFName.c_str() : "";
    String templateName = templateFName.IsValid() == true ? templateFName.c_str() : "";

    String mask = entityName + "_" + templateName + "_";
    uint32 index = 0;
    FastName nameCandidate;
    bool uniqueNameGenerated = false;
    while (uniqueNameGenerated != true)
    {
        uniqueNameGenerated = true;
        nameCandidate = FastName(Format("%s%u", mask.c_str(), index));
        if (reservedName.count(nameCandidate) > 0)
        {
            uniqueNameGenerated = false;
        }
        else
        {
            for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
            {
                SlotComponent* comp = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
                if (comp == component)
                {
                    continue;
                }

                if (comp->GetSlotName() == nameCandidate)
                {
                    uniqueNameGenerated = false;
                    break;
                }
            }
        }

        index++;
    }

    DVASSERT(uniqueNameGenerated == true);

    return nameCandidate;
}

void EditorSlotSystem::DetachEntity(SlotComponent* component, Entity* entity)
{
    Entity* slotEntity = component->GetEntity();
    Entity* loadedEntity = GetScene()->slotSystem->LookUpLoadedEntity(component);
    DVASSERT(loadedEntity == entity);
    DVASSERT(slotEntity == entity->GetParent());

    slotEntity->RemoveNode(entity);
}

void EditorSlotSystem::AttachEntity(SlotComponent* component, Entity* entity, FastName itemName)
{
    Scene* scene = GetScene();

    SlotSystem* slotSystem = GetScene()->slotSystem;
    scene->GetSystem<SelectionSystem>()->SetLocked(true);
    slotSystem->AttachEntityToSlot(component, entity, itemName);
    scene->GetSystem<SelectionSystem>()->SetLocked(false);
}

RefPtr<Entity> EditorSlotSystem::AttachEntity(SlotComponent* component, FastName itemName)
{
    GetScene()->GetSystem<SelectionSystem>()->SetLocked(true);
    SCOPE_EXIT
    {
        GetScene()->GetSystem<SelectionSystem>()->SetLocked(false);
    };

    SlotSystem* slotSystem = GetScene()->slotSystem;
    if (itemName == emptyItemName)
    {
        RefPtr<Entity> newEntity(new Entity());
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
    bool autoGenerateName = accessor->GetGlobalContext()->GetData<SlotSystemSettings>()->autoGenerateSlotNames;
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    auto changeSlotVisitor = [&](const SetFieldValueCommand* cmd)
    {
        const Reflection::Field& field = cmd->GetField();
        ReflectedObject object = field.ref.GetDirectObject();
        FastName fieldName = field.key.Cast<FastName>(FastName(""));
        const ReflectedType* type = object.GetReflectedType();
        if (type == ReflectedTypeDB::Get<SlotComponent>())
        {
            SlotComponent* component = object.GetPtr<SlotComponent>();
            if (fieldName == SlotComponent::ConfigPathFieldName)
            {
                RefPtr<Entity> newEntity(new Entity());
                holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, component, newEntity.Get(), emptyItemName));
            }
            else if (autoGenerateName == true && fieldName == SlotComponent::TemplateFieldName)
            {
                Reflection componentRef = Reflection::Create(ReflectedObject(component));
                Reflection::Field f;
                f.key = SlotComponent::SlotNameFieldName;
                f.ref = componentRef.GetField(f.key);
                DVASSERT(f.ref.IsValid());
                FastName uniqueName = GenerateUniqueSlotName(component, component->GetEntity(), cmd->GetNewValue().Cast<FastName>(), FastName(), Set<FastName>());
                holder.AddPostCommand(std::make_unique<SetFieldValueCommand>(f, uniqueName));
            }
        }
        else if (autoGenerateName == true && type == ReflectedTypeDB::Get<Entity>() && fieldName == Entity::EntityNameFieldName)
        {
            Entity* entityWithNewName = object.GetPtr<Entity>();
            Set<FastName> reservedNames;
            for (uint32 i = 0; i < entityWithNewName->GetComponentCount(Component::SLOT_COMPONENT); ++i)
            {
                SlotComponent* component = static_cast<SlotComponent*>(entityWithNewName->GetComponent(Component::SLOT_COMPONENT, i));
                Reflection componentRef = Reflection::Create(ReflectedObject(component));
                Reflection::Field f;
                f.key = SlotComponent::SlotNameFieldName;
                f.ref = componentRef.GetField(f.key);
                DVASSERT(f.ref.IsValid());
                FastName uniqueName = GenerateUniqueSlotName(component, component->GetEntity(), FastName(), cmd->GetNewValue().Cast<FastName>(), reservedNames);
                reservedNames.insert(uniqueName);
                holder.AddPostCommand(std::make_unique<SetFieldValueCommand>(f, uniqueName));
            }
        }
    };

    const RECommandNotificationObject& commandInfo = holder.GetMasterCommandInfo();
    commandInfo.ForEach<SetFieldValueCommand>(changeSlotVisitor);

    auto removeSlotVisitor = [&](const RemoveComponentCommand* cmd)
    {
        Component* component = const_cast<Component*>(cmd->GetComponent());
        if (component->GetType() == Component::SLOT_COMPONENT)
        {
            SlotComponent* slotComponent = static_cast<SlotComponent*>(component);
            holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, slotComponent, nullptr, FastName()));
        }
    };

    commandInfo.ForEach<RemoveComponentCommand>(removeSlotVisitor);

    auto removeEntityVisitor = [&](const EntityRemoveCommand* cmd)
    {
        Entity* entityToRemove = cmd->GetEntity();
        EditorSlotSystemDetail::DetachSlotForRemovingEntity(entityToRemove, scene, holder);
    };

    commandInfo.ForEach<EntityRemoveCommand>(removeEntityVisitor);

    auto loadDefaultItem = [&](SlotComponent* component)
    {
        FastName itemName = GetSuitableItemName(component);
        if (itemName.IsValid() == false)
        {
            RefPtr<Entity> newEntity(new Entity());
            holder.AddPostCommand(std::unique_ptr<Command>(new AttachEntityToSlot(scene, component, newEntity.Get(), emptyItemName)));
        }
        else
        {
            holder.AddPostCommand(std::unique_ptr<Command>(new AttachEntityToSlot(scene, component, itemName)));
        }
    };

    auto addSlotVisitor = [&](const AddComponentCommand* cmd)
    {
        Component* component = cmd->GetComponent();
        if (component->GetType() == Component::SLOT_COMPONENT)
        {
            SlotSystemSettings* settings = accessor->GetGlobalContext()->GetData<SlotSystemSettings>();
            SlotComponent* slotComponent = static_cast<SlotComponent*>(component);
            if (slotComponent->GetConfigFilePath().IsEmpty())
            {
                slotComponent->SetConfigFilePath(settings->lastConfigPath);
            }
            loadDefaultItem(static_cast<SlotComponent*>(component));

            if (autoGenerateName == true)
            {
                Reflection componentRef = Reflection::Create(ReflectedObject(component));
                Reflection::Field f;
                f.key = SlotComponent::SlotNameFieldName;
                f.ref = componentRef.GetField(f.key);
                DVASSERT(f.ref.IsValid());
                FastName uniqueName = GenerateUniqueSlotName(slotComponent, cmd->GetEntity(), FastName(), FastName(), Set<FastName>());
                holder.AddPostCommand(std::make_unique<SetFieldValueCommand>(f, uniqueName));
            }
        }
    };

    commandInfo.ForEach<AddComponentCommand>(addSlotVisitor);

    auto addEntityVisitor = [&](const EntityAddCommand* cmd)
    {
        Entity* entityForAdd = cmd->GetEntity();

        auto iter = clonedEntityes.find(entityForAdd);
        if (iter != clonedEntityes.end())
        {
            Vector<Entity*> slotEntityes;
            entityForAdd->GetChildEntitiesWithComponent(slotEntityes, Component::SLOT_COMPONENT);
            slotEntityes.push_back(entityForAdd);

            for (Entity* e : slotEntityes)
            {
                for (uint32 i = 0; i < e->GetComponentCount(Component::SLOT_COMPONENT); ++i)
                {
                    loadDefaultItem(static_cast<SlotComponent*>(e->GetComponent(Component::SLOT_COMPONENT, i)));
                }
            }

            clonedEntityes.erase(iter);
        }
    };

    commandInfo.ForEach<EntityAddCommand>(addEntityVisitor);
}

void EditorSlotSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    auto visitor = [&](const SetFieldValueCommand* cmd)
    {
        const Reflection::Field& field = cmd->GetField();
        ReflectedObject object = field.ref.GetDirectObject();
        FastName fieldName = field.key.Cast<FastName>(FastName(""));
        const ReflectedType* type = object.GetReflectedType();
        if (type == ReflectedTypeDB::Get<SlotComponent>())
        {
            SlotComponent* component = object.GetPtr<SlotComponent>();
            if (fieldName == SlotComponent::SlotNameFieldName)
            {
                Entity* entity = scene->slotSystem->LookUpLoadedEntity(component);
                if (entity != nullptr)
                {
                    entity->SetName(component->GetSlotName());
                }
            }
            else if (fieldName == SlotComponent::ConfigPathFieldName)
            {
                SlotSystemSettings* settings = accessor->GetGlobalContext()->GetData<SlotSystemSettings>();
                settings->lastConfigPath = component->GetConfigFilePath();
            }
        }

        if (type == ReflectedTypeDB::Get<Entity>() && fieldName == Entity::EntityNameFieldName)
        {
            Entity* entity = object.GetPtr<Entity>();
            SlotComponent* component = scene->slotSystem->LookUpSlot(entity);
            if (component != nullptr)
            {
                component->SetSlotName(entity->GetName());
            }
        }
    };

    commandNotification.ForEach<SetFieldValueCommand>(visitor);
}

FastName EditorSlotSystem::GetSuitableItemName(SlotComponent* component) const
{
    FastName itemName;

    Vector<SlotSystem::ItemsCache::Item> items = GetScene()->slotSystem->GetItems(component->GetConfigFilePath());
    FastName templateName = component->GetTemplateName();
    if (templateName.IsValid())
    {
        auto iter = std::find_if(items.begin(), items.end(), [templateName](const SlotSystem::ItemsCache::Item& item) {
            return item.type == templateName;
        });

        if (iter != items.end())
        {
            itemName = iter->itemName;
        }
    }

    return itemName;
}

void EditorSlotSystem::Draw()
{
    DataContext* globalCtx = accessor->GetGlobalContext();
    SlotTemplatesData* data = globalCtx->GetData<SlotTemplatesData>();
    Scene* scene = GetScene();

    SlotSystemSettings* settings = globalCtx->GetData<SlotSystemSettings>();
    Color boxColor = settings->slotBoxColor;
    Color boxEdgeColor = settings->slotBoxEdgesColor;
    Color pivotColor = settings->slotPivotColor;

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
                    rh->DrawIcosahedron(pivot, settings->pivotPointSize, pivotColor, RenderHelper::DRAW_SOLID_DEPTH);
                }
            }
        }
    }
}

std::unique_ptr<Command> EditorSlotSystem::PrepareForSave(bool /*saveForGame*/)
{
    if (entities.empty())
    {
        return nullptr;
    }

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    SlotSystem* slotSystem = sceneEditor->slotSystem;

    RECommandBatch* batchCommand = new RECommandBatch("Prepare for save", static_cast<uint32>(entities.size()));
    Map<int32, Vector<Entity*>, std::greater<int32>> depthEntityMap;
    for (Entity* entity : entities)
    {
        int32 depth = 0;
        Entity* parent = entity;
        while (parent != nullptr)
        {
            ++depth;
            parent = parent->GetParent();
        }
        depthEntityMap[depth].push_back(entity);
    }
    for (const auto& entityNode : depthEntityMap)
    {
        for (Entity* entity : entityNode.second)
        {
            for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
            {
                SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
                batchCommand->Add(std::make_unique<AttachEntityToSlot>(sceneEditor, component, nullptr, FastName()));
            }
        }
    }

    return std::unique_ptr<Command>(batchCommand);
}

void EditorSlotSystem::SetScene(Scene* scene)
{
    {
        Scene* currentScene = GetScene();
        if (currentScene != nullptr)
        {
            currentScene->GetSystem<EntityModificationSystem>()->RemoveDelegate(this);
        }
    }

    SceneSystem::SetScene(scene);

    {
        Scene* currentScene = GetScene();
        if (currentScene != nullptr)
        {
            currentScene->GetSystem<EntityModificationSystem>()->AddDelegate(this);
        }
    }
}

void EditorSlotSystem::LoadSlotsPresetImpl(Entity* entity, RefPtr<KeyedArchive> archive)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    FilePath scenePath = scene->GetScenePath();

    SerializationContext serializeCtx;
    serializeCtx.SetVersion(VersionInfo::Instance()->GetCurrentVersion().version);
    serializeCtx.SetScenePath(scenePath.GetDirectory());
    serializeCtx.SetRootNodePath(scenePath);
    serializeCtx.SetScene(scene);

    uint32 slotsCount = archive->GetUInt32("slotsCount", 0);
    if (slotsCount > 0)
    {
        UnorderedMap<FastName, Deque<SlotComponent*>> existsComponents;
        for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
            existsComponents[component->GetSlotName()].push_back(component);
        }

        for (uint32 slotArhcIndex = 0; slotArhcIndex < slotsCount; ++slotArhcIndex)
        {
            KeyedArchive* slotArch = archive->GetArchive(Format("slot_%u", slotArhcIndex));
            DVASSERT(slotArch != nullptr);
            FastName slotName = slotArch->GetFastName("slotName");
            DVASSERT(slotName.IsValid() == true);

            auto iter = existsComponents.find(slotName);
            if (iter != existsComponents.end())
            {
                Deque<SlotComponent*>& components = iter->second;
                if (components.empty() == false)
                {
                    SlotComponent* component = components.front();
                    scene->Exec(std::make_unique<AttachEntityToSlot>(scene, component, nullptr, FastName()));
                    scene->Exec(std::make_unique<RemoveComponentCommand>(entity, component));
                    components.pop_front();
                }
            }

            SlotComponent* newComponent = new SlotComponent();
            newComponent->Deserialize(slotArch, &serializeCtx);
            scene->Exec(std::make_unique<AddComponentCommand>(entity, newComponent));
            FastName loadedItem = slotArch->GetFastName("loadedItem");
            if (loadedItem.IsValid() == true)
            {
                scene->Exec(std::make_unique<AttachEntityToSlot>(scene, newComponent, loadedItem));
            }
        }
    }

    uint32 subEntitiesCount = archive->GetUInt32("subEntitiesCount", 0);
    if (subEntitiesCount > 0)
    {
        UnorderedMap<FastName, Deque<Entity*>> childEntities;
        for (int32 childIndex = 0; childIndex < entity->GetChildrenCount(); ++childIndex)
        {
            Entity* child = entity->GetChild(childIndex);
            DVASSERT(child->GetName().IsValid());
            childEntities[child->GetName()].push_back(child);
        }

        for (uint32 subEntityIndex = 0; subEntityIndex < subEntitiesCount; ++subEntityIndex)
        {
            KeyedArchive* subEntityArch = archive->GetArchive(Format("subEntity_%u", subEntityIndex));
            DVASSERT(subEntityArch != nullptr);
            FastName entityName = subEntityArch->GetFastName("entityName");
            auto iter = childEntities.find(entityName);
            if (iter != childEntities.end())
            {
                Entity* e = iter->second.front();
                iter->second.pop_front();
                LoadSlotsPresetImpl(e, RefPtr<KeyedArchive>::ConstructWithRetain(subEntityArch));
            }
            else
            {
                Logger::Warning("Entity with name \"%s\" in \"%s\" not found. Preset's subtree will be ignored", entityName.c_str(), entity->GetName().c_str());
            }
        }
    }
}
} // namespace DAVA
