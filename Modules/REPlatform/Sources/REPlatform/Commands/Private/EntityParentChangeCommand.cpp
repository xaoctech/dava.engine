#include "REPlatform/Commands/EntityParentChangeCommand.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace EntityParentChangeCommandDetails
{
Matrix4 ConvertLocalTransform(Entity* entity, Entity* parent)
{
    Matrix4 parentInverse = parent->GetWorldTransform();
    parentInverse.Inverse();
    return entity->GetWorldTransform() * parentInverse;
}
}

EntityParentChangeCommand::EntityParentChangeCommand(Entity* _entity, Entity* _newParent, bool saveEntityPosition, Entity* _newBefore /* = nullptr */)
    : RECommand("Move entity")
    , entity(_entity)
    , newParent(_newParent)
    , newBefore(_newBefore)
    , saveEntityPosition(saveEntityPosition)
{
    SafeRetain(entity);

    DVASSERT(entity != nullptr);
    DVASSERT(newParent != nullptr);

    oldParent = entity->GetParent();

    DVASSERT(oldParent != nullptr);
    oldBefore = oldParent->GetNextChild(entity);

    oldTransform = entity->GetLocalTransform();
    newTransform = EntityParentChangeCommandDetails::ConvertLocalTransform(entity, newParent);
}

EntityParentChangeCommand::~EntityParentChangeCommand()
{
    SafeRelease(entity);
}

void EntityParentChangeCommand::Undo()
{
    if (oldBefore == nullptr)
    {
        oldParent->AddNode(entity);
    }
    else
    {
        oldParent->InsertBeforeNode(entity, oldBefore);
    }

    if (saveEntityPosition == true)
    {
        entity->SetLocalTransform(oldTransform);
    }
}

void EntityParentChangeCommand::Redo()
{
    if (newBefore == nullptr)
    {
        newParent->AddNode(entity);
    }
    else
    {
        newParent->InsertBeforeNode(entity, newBefore);
    }

    if (saveEntityPosition == true)
    {
        entity->SetLocalTransform(newTransform);
    }
}

Entity* EntityParentChangeCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityParentChangeCommand)
{
    ReflectionRegistrator<EntityParentChangeCommand>::Begin()
    .End();
}
} // namespace DAVA
