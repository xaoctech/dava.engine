#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SingletonComponent.h>

namespace DAVA
{
class Entity;
}

class GameCollisionSingleComponent : public DAVA::SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(GameCollisionSingleComponent, DAVA::SingletonComponent);

public:
    void SetCollision(DAVA::Entity* entity1, DAVA::Entity* entity2);
    const DAVA::UnorderedSet<DAVA::Entity*>& GetCollisions(DAVA::Entity* entity) const;
    void Clear() override;

private:
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::UnorderedSet<DAVA::Entity*>> collisions;
};
