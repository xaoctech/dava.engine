#pragma once

#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
class Entity;
} // namespace DAVA

enum class eIconMatchPriority
{
    STATIC_OCCLUSION,
    PARTICLE_EFFECT,
    LANDSCAPE,
    LOD,
    SWITCH,
    VEGETATION,
    SKELETON,
    RENDER_OBJECT,
    USER_COMPONENT,
    CAMERA,
    LIGHT,
    WIND,
    PATH,
    TEXT,
    EMPTY
};

namespace Metas
{
class IconMatchPriority
{
public:
    IconMatchPriority(eIconMatchPriority priority_)
        : priority(priority_)
    {
    }

    eIconMatchPriority priority;
};
} // namespace Metas

namespace M
{
using IconMatchPriority = DAVA::Meta<::Metas::IconMatchPriority>;
} // namespace M

class EntityTypeCreatorBase : public DAVA::ReflectionBase
{
public:
    ~EntityTypeCreatorBase() = default;

private:
    DAVA_VIRTUAL_REFLECTION(EntityTypeCreatorBase);
};

class EntityTypeCreator : public EntityTypeCreatorBase
{
public:
    virtual ~EntityTypeCreator() = default;

    virtual DAVA::Entity* CreateEntity() = 0;
    virtual bool IsComponentsMatched(DAVA::Entity* entity) const = 0;
    virtual QString GetEntityTypeName() const = 0;
    virtual QIcon GetIcon() const = 0;

private:
    DAVA_VIRTUAL_REFLECTION(EntityTypeCreator, EntityTypeCreatorBase);
};

class EntityMultipleTypesCreator : public EntityTypeCreatorBase
{
public:
    virtual QString GetCommonTypesName() const = 0;
    virtual DAVA::Vector<EntityTypeCreator*> CreateEntityTypeCreators() const = 0;

private:
    DAVA_VIRTUAL_REFLECTION(EntityMultipleTypesCreator, EntityTypeCreatorBase);
};
