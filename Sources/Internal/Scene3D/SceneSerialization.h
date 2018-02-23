#pragma once

namespace DAVA
{
class Entity;
class KeyedArchive;
class SerializationContext;
class SceneSerialization
{
public:
    enum eSaveLoadType
    {
        LEVEL,
        PREFAB,
    };

    static void SaveHierarchy(Entity* entity, KeyedArchive* keyedArchive, SerializationContext* serializationContext, eSaveLoadType type);
    static Entity* LoadHierarchy(Entity* parent, KeyedArchive* keyedArchive, SerializationContext* serializationContext, eSaveLoadType type);

    static void SaveEntity(Entity* entity, KeyedArchive* archive, SerializationContext* serializationContext, eSaveLoadType type);
    static void LoadEntity(Entity* entity, KeyedArchive* archive, SerializationContext* serializationContext, eSaveLoadType type);
};
}
