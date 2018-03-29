#include "Scene3D/SceneSerialization.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/RuntimeEntityMarkComponent.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Base/ObjectFactory.h"

namespace DAVA
{
void SceneSerialization::SaveHierarchy(Entity* entity, KeyedArchive* keyedArchive, SerializationContext* serializationContext, eSaveLoadType type)
{
    SceneSerialization::SaveEntity(entity, keyedArchive, serializationContext, type);

    uint32 childrenCount = entity->GetChildrenCount();
    Vector<Entity*> entitiesForSave;
    entitiesForSave.reserve(childrenCount);

    for (uint32 i = 0; i < childrenCount; ++i)
    {
        Entity* child = entity->GetChild(i);
        if (child->GetComponent<RuntimeEntityMarkComponent>() == nullptr)
        {
            entitiesForSave.push_back(child);
        }
    }

    keyedArchive->SetInt32("#childrenCount", static_cast<int32>(entitiesForSave.size()));
    for (size_t i = 0; i < entitiesForSave.size(); ++i)
    {
        Entity* child = entitiesForSave[i];
        KeyedArchive* childArchive = new KeyedArchive();
        SaveHierarchy(child, childArchive, serializationContext, type);
        keyedArchive->SetArchive(Format("#child_%d", static_cast<int32>(i)), childArchive);
        SafeRelease(childArchive);
    }
}

Entity* SceneSerialization::LoadHierarchy(Entity* parent, KeyedArchive* keyedArchive, SerializationContext* serializationContext, eSaveLoadType type)
{
    bool resultLoad = true;
    bool keepUnusedQualityEntities = QualitySettingsSystem::Instance()->GetKeepUnusedEntities();

    Entity* entity = new Entity();
    SceneSerialization::LoadEntity(entity, keyedArchive, serializationContext, type);
    if ((keepUnusedQualityEntities || QualitySettingsSystem::Instance()->IsQualityVisible(entity)))
    {
        if (parent)
            parent->AddNode(entity);
    }

    ParticleEffectComponent* effect = entity->GetComponent<ParticleEffectComponent>();
    if (effect && (effect->loadedVersion == 0))
        effect->CollapseOldEffect(serializationContext);

    int32 childrenCount = keyedArchive->GetInt32("#childrenCount", 0);
    entity->children.reserve(childrenCount);
    for (int ci = 0; ci < childrenCount; ++ci)
    {
        KeyedArchive* childArchive = keyedArchive->GetArchive(Format("#child_%d", ci));
        Entity* child = LoadHierarchy(entity, childArchive, serializationContext, type);
        SafeRelease(child);
    }
    return entity;
}

void SceneSerialization::SaveEntity(Entity* entity, KeyedArchive* archive, SerializationContext* serializationContext, eSaveLoadType type)
{
    archive->SetString("name", String(entity->name.c_str()));
    archive->SetUInt32("id", entity->id);
    archive->SetUInt32("flags", entity->flags);

    KeyedArchive* compsArch = new KeyedArchive();
    uint32 savedIndex = 0;
    for (Component* c : entity->components)
    {
        const ReflectedType* refType = ReflectedTypeDB::GetByType(c->GetType());
        DVASSERT(refType != nullptr);

        ReflectedMeta* meta = refType->GetStructure()->meta.get();
        DVASSERT(meta != nullptr);

        bool isSerializable = meta->GetMeta<M::NonSerializableComponent>() == nullptr;

        if (isSerializable)
        {
            //don't save empty custom properties
            if (c->GetType()->Is<CustomPropertiesComponent>())
            {
                CustomPropertiesComponent* customProps = CastIfEqual<CustomPropertiesComponent*>(c);
                if (customProps && customProps->GetArchive()->Count() <= 0)
                {
                    continue;
                }
            }

            KeyedArchive* compArch = new KeyedArchive();
            c->Serialize(compArch, serializationContext);
            compsArch->SetArchive(KeyedArchive::GenKeyFromIndex(savedIndex), compArch);
            compArch->Release();
            savedIndex++;
        }
    }

    compsArch->SetUInt32("count", savedIndex);
    archive->SetArchive("components", compsArch);
    compsArch->Release();
}

void SceneSerialization::LoadEntity(Entity* entity, KeyedArchive* archive, SerializationContext* serializationContext, eSaveLoadType type)
{
    entity->name = FastName(archive->GetString("name", "").c_str());
    entity->id = archive->GetUInt32("id", 0);
    entity->flags = archive->GetUInt32("flags", Entity::NODE_VISIBLE);
    entity->flags &= ~Entity::TRANSFORM_DIRTY;

    KeyedArchive* compsArch = archive->GetArchive("components");
    if (nullptr != compsArch)
    {
        uint32 componentCount = compsArch->GetUInt32("count");
        for (uint32 i = 0; i < componentCount; ++i)
        {
            KeyedArchive* compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            if (nullptr != compArch)
            {
                String componentType = compArch->GetString("comp.typename");

                /*
                 
                 const ReflectedType* rType = ReflectedTypeDB::GetByPermanentName(componentType);
                 Any anyObj = rType->CreateObject(ReflectedType::CreatePolicy::ByPointer);
                 Component* comp = anyObj.Cast<Component*>();
                */

                Component* comp = ObjectFactory::Instance()->New<Component>(componentType);
                if (nullptr != comp)
                {
                    if (comp->GetType()->Is<TransformComponent>())
                    {
                        entity->RemoveComponent(comp->GetType());
                    }

                    entity->AddComponent(comp);
                    comp->Deserialize(compArch, serializationContext);
                }
            }
        }
    }
}
}
