#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class EntityParentChangeCommand : public RECommand
{
public:
    EntityParentChangeCommand(Entity* entity, Entity* newParent, Entity* newBefore = NULL);
    ~EntityParentChangeCommand();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    Entity* entity;
    Entity* oldParent;
    Entity* oldBefore;
    Entity* newParent;
    Entity* newBefore;

private:
    DAVA_VIRTUAL_REFLECTION(EntityParentChangeCommand, RECommand);
};
} // namespace DAVA
