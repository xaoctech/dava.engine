#pragma once

#include "Commands2/Base/RECommand.h"

namespace DAVA
{
class Entity;
}

class EntityAddCommand : public RECommand
{
public:
    EntityAddCommand(DAVA::Entity* entityToAdd, DAVA::Entity* parentToAdd,
                     DAVA::Entity* insertBefore = nullptr);
    ~EntityAddCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;

    DAVA::Entity* entityToAdd = nullptr;
    DAVA::Entity* parentToAdd = nullptr;
    DAVA::Entity* insertBefore = nullptr;
};
