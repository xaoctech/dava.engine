#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SingleComponent.h>

namespace DAVA
{
class Entity;
}

class GameCollisionSingleComponent : public DAVA::ClearableSingleComponent
{
    DAVA_VIRTUAL_REFLECTION(GameCollisionSingleComponent, DAVA::ClearableSingleComponent);

public:
    GameCollisionSingleComponent();
    void SetCollision(DAVA::Entity* entity1, DAVA::Entity* entity2);
    const DAVA::UnorderedSet<DAVA::Entity*>& GetCollisions(DAVA::Entity* entity) const;

private:
    void Clear() override;

private:
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::UnorderedSet<DAVA::Entity*>> collisions;
};
