#pragma once
#include "Commands2/Base/RECommand.h"

namespace DAVA
{
class Entity;
}

class EntityParentChangeCommand : public RECommand
{
public:
    EntityParentChangeCommand(DAVA::Entity* entity, DAVA::Entity* newParent,
                              bool saveEntityPosition,
                              DAVA::Entity* newBefore = nullptr);
    ~EntityParentChangeCommand();

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

    DAVA::Entity* entity = nullptr;
    DAVA::Entity* oldParent = nullptr;
    DAVA::Entity* oldBefore = nullptr;
    DAVA::Entity* newParent = nullptr;
    DAVA::Entity* newBefore = nullptr;
    bool saveEntityPosition;
};
