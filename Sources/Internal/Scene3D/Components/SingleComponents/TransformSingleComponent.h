#pragma once

#include "Base/Vector.h"
#include "Entity/SortedEntityContainer.h"
#include "Entity/SingletonComponent.h"

namespace DAVA
{
class Entity;
class TransformSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(TransformSingleComponent, SingletonComponent);

    Vector<Entity*> localTransformChanged;
    Vector<Entity*> transformParentChanged;
    SortedEntityContainer worldTransformChanged; //sorted by EntityFamily in TransformSystem
    Vector<Entity*> animationTransformChanged;

    void Clear();
    void EraseEntity(const Entity* entity);
};
}
