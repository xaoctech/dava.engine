#pragma once

#include "Base/Vector.h"
#include "Entity/SortedEntityContainer.h"
#include "Entity/SingleComponent.h"

namespace DAVA
{
class Entity;
class TransformSingleComponent : public ClearableSingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(TransformSingleComponent, ClearableSingleComponent);

    TransformSingleComponent();

    Vector<Entity*> localTransformChanged;
    Vector<Entity*> transformParentChanged;
    SortedEntityContainer worldTransformChanged; //sorted by EntityFamily in TransformSystem
    Vector<Entity*> animationTransformChanged;

    void EraseEntity(const Entity* entity);

private:
    void Clear() override;
};
}
