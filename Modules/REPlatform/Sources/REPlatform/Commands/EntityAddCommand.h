#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class EntityAddCommand : public RECommand
{
public:
    EntityAddCommand(Entity* entityToAdd, Entity* toParent);
    ~EntityAddCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

    Entity* entityToAdd;
    Entity* parentToAdd;

private:
    DAVA_VIRTUAL_REFLECTION(EntityAddCommand, RECommand);
};
}
